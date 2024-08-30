#include <stdexcept>
#include <vector>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Path.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/LoopSimplify.h>

#include "IRBasedSourceSpecMatcher.h"
#include "SourceSpecTransformer.h"
#include "SpecDeclarationCollector.h"
#include "IrSpecConstants.h"
#include "IrLookup.h"
#include "MetadataUtils.h"
#include "PallasException.h"
#include "SrcLookup.h"
#include "wrapper/SpecParserWrapper.h"

namespace sourcespec2ir
{
    SourceSpecTransformer::SourceSpecTransformer(
        std::unique_ptr<SourceCompilerDriver> compilerDriver,
        std::unique_ptr<SpecUnifier> specUnifier,
        std::string srcInPath, std::string outPath,
        const utils::OutputConfig &outputConfig,
        const std::string &workingDir,
        bool runMem2Reg)
        : srcInPath(srcInPath),
          outPath(outPath),
          workingDir(workingDir),
          outputConfig(outputConfig),
          runMem2Reg(runMem2Reg),
          srcCompilerDriver(std::move(compilerDriver)),
          specUnifier(std::move(specUnifier))
    {
        // Create global IRContext
        irContext = std::make_unique<llvm::LLVMContext>();

        // Setup the PassBuilder
        passBuilder.registerModuleAnalyses(mam);
        passBuilder.registerCGSCCAnalyses(cgam);
        passBuilder.registerFunctionAnalyses(fam);
        passBuilder.registerLoopAnalyses(lam);
        passBuilder.crossRegisterProxies(lam, fam, cgam, mam);
    }

    void SourceSpecTransformer::transformModule()
    {
        // Compile the source-file into IR
        irModule = srcCompilerDriver->compileSource(srcInPath, *irContext);

        // Simplify loops
        llvm::ModulePassManager mpm;
        mpm.addPass(
            llvm::createModuleToFunctionPassAdaptor(llvm::LoopSimplifyPass()));
        mpm.run(*irModule, mam);

        // Run mem2reg only if cl-option was set.
        if (runMem2Reg)
            promoteMemToReg(*irModule);

        /*
        if (outputConfig.printDebugInfo)
        {
            llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++\n";
            llvm::outs() << "PRE-Transformation\n";
            writeToFile(*irModule, "");
            llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++\n";
        }
        */

        // Parse the source-file to extract the specifications
        auto parser = std::make_unique<SpecParserWrapper>(srcInPath);
        auto srcParseTree = parser->getSrcParseTree();
        if (!srcParseTree)
            throw utils::PallasException("Unable to parse the source file.");

        // Set up the WrapperGenerator
        wrapperGenerator = createWrapperGenerator();

        // Collect declarations
        SpecDeclarationCollector declCollector(srcParseTree);
        auto specDeclarations = declCollector.collectDeclarations();

        // Match the specifications to the IR
        auto specMatcher = createSpecMatcher(srcParseTree, irModule.get(),
                                             srcInPath);
        auto matchedSpecs = specMatcher->matchSpecs();

        // Release the parser and input-streams
        parser.reset();

        // TODO: This should be bundled in the Wrapper generator, and not here
        // Create wrappers for function-contracts
        for (const auto &contract : matchedSpecs->matchedFContracts)
            wrapFunctionContractBlock(*contract);
        // Create wrappers for loop-invariants
        for (const auto &invBlock : matchedSpecs->matchedLoopInvariants)
            wrapLoopInvariantBlock(*invBlock);
        // Create wrappers for blocks of specification-statements
        for (const auto &stmntBlock : matchedSpecs->matchedStmntBlocks)
            wrapSpecStatementBlock(*stmntBlock);

        // Collect pointers to wrapper-functions
        llvm::SmallVector<SrcWrapperFunction *> wrappers;
        for (auto &[contr, wrapper] : fContractWrappers)
            wrappers.push_back(wrapper.get());
        for (auto &[inv, wrapper] : loopInvWrappers)
            wrappers.push_back(wrapper.get());
        for (auto &[stmnt, wrapper] : statementWrappers)
            wrappers.push_back(wrapper.get());

        // Insert wrappers into source
        const auto srcWrapperPath = getPathForSourceWithWrappers();
        wrapperGenerator->buildSrcWithWrappers(srcInPath, srcWrapperPath,
                                               wrappers, *specDeclarations);
        // Compile source
        auto moduleWithWrappers = srcCompilerDriver->compileSource(
            srcWrapperPath, *irContext);

        // Turn all duplicate global values into declarations to avoid conflicts
        // while linking.
        // Potential alternative: Add the specification-metadata directly
        // after the wrapping stage and add reference to wrapper-function later.
        removeDuplicateDefinitions(*irModule, *moduleWithWrappers);

        // Apply mem2reg to new module
        promoteMemToReg(*moduleWithWrappers);

        auto linker = llvm::Linker(*irModule);
        linker.linkInModule(std::move(moduleWithWrappers));

        // Update the debug-locations in the generated wrapper-functions
        // to match the locations in the original source-file.
        rewriteWrapperLocations(*matchedSpecs);

        if (outputConfig.printDebugInfo)
        {
            llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++\n";
            llvm::outs() << "PRE-Unification\n";
            writeToFile(*irModule, "");
            llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++\n";
        }

        // Generate metadata for specifications
        for (const auto &contract : matchedSpecs->matchedFContracts)
            encodeFunctionContractBlock(*contract);
        for (const auto &invBlock : matchedSpecs->matchedLoopInvariants)
            encodeLoopInvBlock(*invBlock);
        for (const auto &stmntBlock : matchedSpecs->matchedStmntBlocks)
            encodeStmntBlock(*stmntBlock);

        // Check wellformedness of module
        verifyModule(*irModule, "After transformation");

        // Transform the compiled specifications into common format
        // (calls to specification-functions)
        specUnifier->unifySpecifications(irModule.get(), irContext.get());

        // Check wellformedness of module
        verifyModule(*irModule, "After unification");

        // Dump the generated IR
        writeToFile(*irModule, outPath);
    }

    // TODO: Move this into the wrapper-generator
    // i.e. The wrapper generator should construct the wrapping functions and
    // return them back to the SourceSpecTransfromer for matching after the
    // compilation.
    void SourceSpecTransformer::wrapFunctionContractBlock(
        const MatchedFunctionContract &contract)
    {
        const auto parentFunction = contract.function;
        auto argList = utils::getSrcVariablesForFunction(*parentFunction);
        // Get insert-position for wrapper-functions
        auto insertPos = wrapperGenerator->getWrapperInsertLoc(contract);

        // Wrap specification statements
        for (auto &stmnt : contract.statements)
        {
            auto wrapperName = wrapperGenerator->getNameForWrapperFunction();
            auto wrapperStr = wrapperGenerator->getWrapperFunctionString(
                argList->getArrayRef(), stmnt->expr, wrapperName);
            fContractWrappers[stmnt.get()] = std::make_unique<SrcWrapperFunction>(
                wrapperName, wrapperStr, argList->getArrayRef(), insertPos);
        }
    }

    // TODO: Move this into the wrapper-generator
    void SourceSpecTransformer::wrapLoopInvariantBlock(
        const MatchedLoopInvariantBlock &invBlock)
    {
        // Determine the variables that can be referenced in the invariant
        auto argList = utils::getVarsForLoopInv(
            invBlock.parentFunction, invBlock.loopHeader);
        auto insertPos = wrapperGenerator->getWrapperInsertLoc(invBlock);

        // Generate wrappers for the individual invariants
        for (auto &inv : invBlock.invariants)
        {
            auto wrapperName = wrapperGenerator->getNameForWrapperFunction();
            auto wrapperStr = wrapperGenerator->getWrapperFunctionString(
                argList->getArrayRef(), inv->expr, wrapperName);
            loopInvWrappers[inv.get()] = std::make_unique<SrcWrapperFunction>(
                wrapperName, wrapperStr, argList->getArrayRef(), insertPos);
        }
    }

    void SourceSpecTransformer::wrapSpecStatementBlock(
        const MatchedSpecStatementBlock &stmntBlock)
    {
        // Determine the variables that can be referenced in the statement
        auto argList = utils::getVarsForStatement(stmntBlock.inst);
        // Get insert-location for wrapper-function
        auto insertPos = wrapperGenerator->getWrapperInsertLoc(stmntBlock);

        // Generate wrapper for the individual statements
        for (auto &stmnt : stmntBlock.statements)
        {
            auto wrapperName = wrapperGenerator->getNameForWrapperFunction();
            auto wrapperStr = wrapperGenerator->getWrapperFunctionString(
                argList->getArrayRef(), stmnt->expr, wrapperName);
            statementWrappers[stmnt.get()] = std::make_unique<SrcWrapperFunction>(
                wrapperName, wrapperStr, argList->getArrayRef(), insertPos);
        }
    }

    void SourceSpecTransformer::encodeLoopInvBlock(
        const MatchedLoopInvariantBlock &invBlock)
    {
        const auto numInvs = invBlock.invariants.size();
        if (numInvs == 0)
            return;

        // Vector of MD-nodes that makes up the tuple with the invariants
        llvm::SmallVector<llvm::Metadata *> invNodes;

        // MD-String that identifies the tuple as a set of loop-invariants
        auto invIDNode = llvm::MDString::get(*irContext,
                                             PALLAS_LOOP_INV_BLOCK);
        invNodes.push_back(invIDNode);

        // Add info on srcLocation
        invNodes.push_back(encodeSrcLoc(invBlock.srcLoc));

        // Transform the individual invariants and add them to invNodes
        for (auto &inv : invBlock.invariants)
            encodeLoopInv(*inv, invNodes);

        // Construct MD-tuple and add it to the loop's llvm.loop MD-node
        auto loopMDTuple = invBlock.loopID;
        auto invNode = llvm::MDTuple::get(*irContext, invNodes);
        loopMDTuple->push_back(invNode);
    }

    void SourceSpecTransformer::encodeLoopInv(
        const MatchedLoopInvariant &inv,
        llvm::SmallVector<llvm::Metadata *> &nodes)
    {
        // Get the ir-version of the wrapper-function
        auto srcWrapperFun = loopInvWrappers.at(&inv).get();
        auto irWrapperFunction = utils::getFunctionBySrcName(
            *irModule, srcWrapperFun->name);

        // Add a metadata-marker to the generated function
        utils::markGeneratedSpecFunction(*irWrapperFunction, *irContext);

        // Assemble MD-node to represent invariant-statement
        llvm::SmallVector<llvm::Metadata *> mdElems;
        // Add source-location info
        mdElems.push_back(encodeSrcLoc(inv.srcLoc));

        // Add ref to wrapper-function
        mdElems.push_back(llvm::ValueAsMetadata::get(irWrapperFunction));
        // Add references to the metadata-nodes that describe how the arguments
        // of the specification-function relate to the original variables.
        for (auto arg : srcWrapperFun->argList)
            mdElems.push_back(arg);

        // Build new node and add it to the vector of the inv-block.
        auto invAsMd = llvm::MDNode::get(*irContext, mdElems);
        nodes.push_back(invAsMd);
    }

    void SourceSpecTransformer::encodeStmntBlock(
        const MatchedSpecStatementBlock &stmntBlock)
    {
        const auto inst = stmntBlock.inst;
        if (stmntBlock.inst == nullptr)
            throw utils::PallasException("Encountered unmatched stmnt-block");

        // Transform specification statements
        llvm::SmallVector<llvm::Metadata *> stmntNodes;

        // Add location-information
        stmntNodes.push_back(encodeSrcLoc(stmntBlock.srcLoc));

        for (auto &stmnt : stmntBlock.statements)
            stmntNodes.push_back(encodeSpecStatement(*stmnt));

        // Build md-node to attach to function
        auto blockMDNode = llvm::MDTuple::get(*irContext, stmntNodes);
        inst->setMetadata(PALLAS_STMNT_BLOCK, blockMDNode);
    }

    llvm::MDNode *SourceSpecTransformer::encodeSpecStatement(
        const MatchedSpecStatement &stmnt)
    {
        auto srcWrapperFun = statementWrappers.at(&stmnt).get();
        auto irWrapperFun = utils::getFunctionBySrcName(
            *irModule, srcWrapperFun->name);

        // Add a metadata-marker to generated specification-function
        utils::markGeneratedSpecFunction(*irWrapperFun, *irContext);

        // Generate the metadata that encodes the contract statement:
        llvm::SmallVector<llvm::Metadata *, 2> mdElems;
        // Add statement identifier
        std::string mdId;
        if (stmnt.type == Assert)
            mdId = PALLAS_ASSERT;
        else if (stmnt.type == Assume)
            mdId = PALLAS_ASSUME;
        else
            throw utils::PallasException("Unsupported statement-type");
        mdElems.push_back(llvm::MDString::get(*irContext, mdId));

        // Add info on srcLocation
        mdElems.push_back(encodeSrcLoc(stmnt.srcLoc));

        // Add reference to function
        mdElems.push_back(llvm::ValueAsMetadata::get(irWrapperFun));

        // Add references to the metadata-nodes that describe how the arguments
        // of the specification-function relate to the original variables.
        for (auto arg : srcWrapperFun->argList)
            mdElems.push_back(arg);

        return llvm::MDNode::get(*irContext, mdElems);
    }

    void SourceSpecTransformer::promoteMemToReg(llvm::Module &module)
    {
        llvm::ModulePassManager mpm;
        mpm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::PromotePass()));
        mpm.run(module, mam);
    }

    void SourceSpecTransformer::verifyModule(
        llvm::Module &module, const std::string msg)
    {
        if (llvm::verifyModule(module, &llvm::errs()))
        {
            std::string eMessage = "Ill-formed LLVM-module (" + msg + ")";
            throw utils::PallasException(eMessage);
        }
    }

    void SourceSpecTransformer::encodeFunctionContractBlock(
        const MatchedFunctionContract &contract)
    {
        const auto parentFunction = contract.function;
        llvm::SmallVector<llvm::Metadata *> contractMDNodes;

        // Add src-location information
        contractMDNodes.push_back(encodeSrcLoc(contract.srcLoc));

        // Add pure-flag
        auto pureFlagConst = llvm::ConstantAsMetadata::get(
            llvm::ConstantInt::getBool(*irContext, contract.isPure));
        contractMDNodes.push_back(pureFlagConst);

        // Transform specification statements
        for (auto &matchedStmnt : contract.statements)
        {
            auto mdNode = encodeFunctionContractStatement(*parentFunction,
                                                          *matchedStmnt);
            contractMDNodes.push_back(mdNode);
        }

        auto contractMDNode = llvm::MDTuple::get(*irContext, contractMDNodes);
        parentFunction->setMetadata(PALLAS_FUN_CONTRACT_BLOCK, contractMDNode);
    }

    llvm::MDNode *SourceSpecTransformer::encodeFunctionContractStatement(
        const llvm::Function &function,
        const MatchedFContractStatement &stmnt)
    {
        // Get the ir-version of the wrapper-function
        auto srcWrapperFun = fContractWrappers.at(&stmnt).get();
        auto irWrapperFunction = utils::getFunctionBySrcName(
            *irModule, srcWrapperFun->name);

        // Add a metadata-marker to generated specification-function
        utils::markGeneratedSpecFunction(*irWrapperFunction, *irContext);

        // Generate the metadata that encodes the contract statement:
        llvm::SmallVector<llvm::Metadata *> mdElems;

        // Add statement identifier
        std::string mdId;
        if (stmnt.type == Requires)
            mdId = PALLAS_REQUIRES;
        else if (stmnt.type == Ensures)
            mdId = PALLAS_ENSURES;
        else
            throw utils::PallasException("Unsupported statement-type");
        auto mdIdentifier = llvm::MDString::get(*irContext, mdId);
        mdElems.push_back(mdIdentifier);

        // Add src-location information
        mdElems.push_back(encodeSrcLoc(stmnt.srcLoc));

        // Add reference to function
        auto specFuncAsMd = llvm::ValueAsMetadata::get(irWrapperFunction);
        mdElems.push_back(specFuncAsMd);

        // Add references to the metadata-nodes that describe how the arguments
        // of the specification-function relate to the original variables.
        for (auto arg : srcWrapperFun->argList)
            mdElems.push_back(arg);

        return llvm::MDNode::get(*irContext, mdElems);
    }

    void SourceSpecTransformer::rewriteWrapperLocations(
        const MatchedSpecifications &matchedSpecs)
    {
        // Handle function-contracts
        for (auto &contr : matchedSpecs.matchedFContracts)
        {
            auto cu = getCUForFunc(contr->function);
            auto scopeFile = getFileForFunc(contr->function);
            for (auto &stmnt : contr->statements)
                rewriteWrapperLocations(*fContractWrappers[stmnt.get()],
                                        stmnt->exprLoc, cu, scopeFile);
        }
        // Handle loop-invariants
        for (auto &invBlock : matchedSpecs.matchedLoopInvariants)
        {
            auto cu = getCUForFunc(invBlock->parentFunction);
            auto scopeFile = getFileForFunc(invBlock->parentFunction);
            for (auto &inv : invBlock->invariants)
                rewriteWrapperLocations(*loopInvWrappers[inv.get()],
                                        inv->exprLoc, cu, scopeFile);
        }
        // Handle specification statements
        for (auto &stmntBlock : matchedSpecs.matchedStmntBlocks)
        {
            auto instr = stmntBlock->inst;
            auto parentFunc = instr->getParent()->getParent();
            auto cu = getCUForFunc(parentFunc);
            auto scopeFile = getFileForFunc(parentFunc);
            for (auto &stmnt : stmntBlock->statements)
                rewriteWrapperLocations(*statementWrappers[stmnt.get()],
                                        stmnt->exprLoc, cu, scopeFile);
        }
    }

    void SourceSpecTransformer::rewriteWrapperLocations(
        const SrcWrapperFunction &wFun, const SrcLocation &exprStartLoc,
        llvm::DICompileUnit *cu, llvm::DIFile *scopeFile)
    {
        // Get the wrapper-function
        auto irWrapperF = utils::getFunctionBySrcName(*irModule, wFun.name);
        if (irWrapperF == nullptr)
            throw utils::PallasException("Failed to get wrapper-function by name");

        auto oldSProg = irWrapperF->getSubprogram();
        auto oldFLine = oldSProg->getLine(); // Needed to calculate new positions for instructions
        llvm::DIBuilder diBuilder(*irModule, true, cu);

        // Update the location of the wrapper-functions DISubprogram
        // The line-info cannot be modified manually, so we replace the full
        // DISubprogram of the wrapper-function.
        auto newSProg = diBuilder.createFunction(
            scopeFile, // New scope
            oldSProg->getName(),
            oldSProg->getLinkageName(),
            scopeFile,         // New file
            exprStartLoc.line, // New line
            oldSProg->getType(),
            exprStartLoc.line, // New scope line
            oldSProg->getFlags(),
            oldSProg->getSPFlags(),
            oldSProg->getTemplateParams(),
            oldSProg->getDeclaration(),
            oldSProg->getThrownTypes(),
            oldSProg->getAnnotations(),
            oldSProg->getTargetFuncName());
        irWrapperF->setSubprogram(newSProg);

        // Update the dbg-locations of the instructions within the function
        auto [lineOffset, colOffset] = wrapperGenerator->getOffsetToExprStart(wFun);
        for (auto it = inst_begin(irWrapperF), end = inst_end(irWrapperF); it != end; ++it)
        {
            auto &oldInstrLoc = it->getDebugLoc();
            if (!oldInstrLoc)
                continue;

            // If the debug-location refers to one of the wrapper function's
            // arguments or a return-instruction, it would go out of scope.
            // In these cases, we drop the location
            if (exprStartLoc.line + oldInstrLoc.getLine() < oldFLine + lineOffset ||
                (oldInstrLoc.getLine() - oldFLine - lineOffset == 0 && oldInstrLoc.getCol() < colOffset))
            {
                // Drop the dbg-location
                it->dropLocation();
            }
            else
            {
                // Update the dbg-location
                auto newLine = exprStartLoc.line + oldInstrLoc.getLine() - oldFLine - lineOffset;
                auto newCol = oldInstrLoc.getCol();
                // If the instruction is on the first-line, we need to add
                // the additional column-offset.
                if (newLine == exprStartLoc.line)
                {
                    newCol += exprStartLoc.column;
                    newCol -= colOffset;
                }

                auto newScope = oldInstrLoc.getScope();
                // Replace references to old DISubprogram
                if (newScope == oldSProg)
                    newScope = newSProg;

                // TODO: handle inlined at, and implicit code
                auto newDebugLoc = llvm::DILocation::get(
                    *irContext, newLine, newCol, newScope);
                it->setDebugLoc(llvm::DebugLoc(newDebugLoc));
            }

            // If the instruction is a call to a debug-intrinsic, update the
            // scope of the DIVariables.
            if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*it))
            {
                auto oldVar = intr->getVariable();
                if (oldVar->isParameter())
                {
                    auto newVar = diBuilder.createParameterVariable(
                        newSProg, // New scope
                        oldVar->getName(),
                        oldVar->getArg(),
                        scopeFile,         // New File
                        exprStartLoc.line, // New line
                        oldVar->getType(),
                        false, // TODO: Check if alwaysPreserve should be true
                        oldVar->getFlags(),
                        oldVar->getAnnotations());
                    intr->setVariable(newVar);
                }
                else
                {
                    auto newVar = diBuilder.createAutoVariable(
                        newSProg, // New scope
                        oldVar->getName(),
                        scopeFile,         // New file
                        exprStartLoc.line, // New line
                        oldVar->getType(),
                        false, // TODO: Check if alwaysPreserve should be true
                        oldVar->getFlags(),
                        oldVar->getAlignInBits());
                    intr->setVariable(newVar);
                }
            }
        }

        diBuilder.finalize();
    }

    void SourceSpecTransformer::writeToFile(
        const llvm::Module &module, std::string outPath)
    {
        if (outPath.empty() || outputConfig.printDebugInfo)
        {
            // Print to console if no output-path is given or if debug-info is enabled
            llvm::outs() << "===============================================\n";
            // irModule->dump();
            module.print(llvm::outs(), nullptr);
            llvm::outs() << "===============================================\n";
        }

        if (!outPath.empty())
        {
            // Write the IR-representation to the given file.
            std::error_code eCode;
            llvm::raw_fd_ostream outStream(outPath, eCode,
                                           llvm::sys::fs::CD_CreateAlways);
            if (!eCode)
            {
                // Use IRPrintingPasses alternatively
                // module.print(outStream, nullptr, false, true);
                module.print(outStream, nullptr);
            }
            else
            {
                throw std::runtime_error("Failed to write output to " +
                                         std::string(outPath) +
                                         ". Error code: " + eCode.message());
            }
            outStream.close();
        }
    }

    void SourceSpecTransformer::removeDuplicateDefinitions(
        const llvm::Module &origModule, llvm::Module &moduleWithWrappers)
    {
        // Collect the GUIDs of the globals in the original module
        // Stores the GUIDs in a sorted vector
        std::vector<llvm::GlobalValue::GUID> origGlobals;
        for (auto &g : origModule.global_values())
            origGlobals.push_back(g.getGUID());
        std::sort(origGlobals.begin(), origGlobals.end());

        // Turn all global definitions from the modified module into declarations
        for (auto &g : moduleWithWrappers.global_values())
        {
            auto id = g.getGUID();
            if (g.isDeclaration() ||
                !std::binary_search(origGlobals.begin(), origGlobals.end(), id))
                continue;
            // Alternative: Set linkage to external
            llvm::convertToDeclaration(g);
            // llvm::outs() << "Turned " << g.getName().str() << " into declaration.\n";
        }
    }

    std::string SourceSpecTransformer::getPathForSourceWithWrappers()
    {
        // Build the file-name using the extension from the source-file
        const auto srcExt = srcInPath.substr(srcInPath.find_last_of(".") + 1);
        const std::string tmpSrcFilename = "source_wrappers." + srcExt;
        // Append the generated file-name to the path to the working-directory
        auto tmpSrcPath = llvm::SmallString<128>(workingDir);
        llvm::sys::path::append(tmpSrcPath, tmpSrcFilename);
        return tmpSrcPath.str().str();
    }

    llvm::MDTuple *SourceSpecTransformer::encodeSrcLoc(const SrcRange &srcLoc)
    {
        auto *nTy = llvm::Type::getInt64Ty(*irContext);

        llvm::SmallVector<llvm::Metadata *> mdElems = {
            llvm::MDString::get(*irContext, PALLAS_SRC_LOCATION_MARKER),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(nTy, srcLoc.startLoc.line)),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(nTy, srcLoc.startLoc.column)),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(nTy, srcLoc.endLoc.line)),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(nTy, srcLoc.endLoc.column))};

        return llvm::MDTuple::get(*irContext, mdElems);
    }

    llvm::DIFile *SourceSpecTransformer::getFileForFunc(llvm::Function *f)
    {
        if (f == nullptr)
            throw utils::PallasException("Passed function is nullptr.");
        if (f->getSubprogram() == nullptr)
            throw utils::PallasException("Encountered a function without a subprogram");
        auto *file = f->getSubprogram()->getFile();
        if (file == nullptr)
            throw utils::PallasException("Encountered a function without a file.");
        return file;
    }

    llvm::DICompileUnit *SourceSpecTransformer::getCUForFunc(llvm::Function *f)
    {
        if (f == nullptr)
            throw utils::PallasException("Passed function is nullptr.");
        if (f->getSubprogram() == nullptr)
            throw utils::PallasException("Encountered a function without a subprogram");
        auto *cu = f->getSubprogram()->getUnit();
        if (cu == nullptr)
            throw utils::PallasException("Encountered a function without a compile-unit.");
        return cu;
    }

    /*
     * Factory methods
     */
    std::unique_ptr<SourceSpecMatcher> SourceSpecTransformer::createSpecMatcher(
        SpecParser::FileContext *specParseTree, llvm::Module *irModule,
        std::string srcFilePath)
    {
        return std::make_unique<IRBasedSourceSpecMatcher>(
            specParseTree, irModule, srcFilePath, &fam,
            outputConfig.printMatchingInfo);
    }
}
