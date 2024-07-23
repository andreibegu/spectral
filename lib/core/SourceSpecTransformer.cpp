#include <stdexcept>
#include <vector>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
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
        const std::string &workingDir)
        : srcInPath(srcInPath),
          outPath(outPath),
          workingDir(workingDir),
          outputConfig(outputConfig),
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

        // Run mem2reg
        // promoteMemToReg(*irModule);

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
        auto specMatcher = createSpecMatcher(srcParseTree, irModule.get());
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

        // Apply mem2reg to new module
        promoteMemToReg(*moduleWithWrappers);

        // Turn all duplicate global values into declarations to avoid conflicts
        // while linking.
        // Potential alternative: Add the specification-metadata directly
        // after the wrapping stage and add reference to wrapper-function later.
        removeDuplicateDefinitions(*irModule, *moduleWithWrappers);

        auto linker = llvm::Linker(*irModule);
        linker.linkInModule(std::move(moduleWithWrappers));

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

        // Transform loop-invariants into IR
        for (const auto &invBlock : matchedSpecs->matchedLoopInvariants)
            encodeLoopInvBlock(*invBlock);

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
        auto srcVars = utils::getSrcVariablesForFunction(*parentFunction);
        // Turn arg-list into sequential container for explicit ordering
        llvm::SmallVector<llvm::DILocalVariable *> argList(
            srcVars->begin(), srcVars->end());
        // Get insert-position for wrapper-functions
        auto insertPos = wrapperGenerator->getWrapperInsertLoc(contract);

        // Wrap specification statements
        for (auto &stmnt : contract.statements)
        {
            auto wrapperName = wrapperGenerator->getNameForWrapperFunction();
            auto wrapperStr = wrapperGenerator->getWrapperFunctionString(
                argList, stmnt->expr, wrapperName);
            fContractWrappers[stmnt.get()] = std::make_unique<SrcWrapperFunction>(
                wrapperName, wrapperStr, argList, insertPos);
        }
    }

    // TODO: Move this into the wrapper-generator
    void SourceSpecTransformer::wrapLoopInvariantBlock(
        const MatchedLoopInvariantBlock &invBlock)
    {
        // Determine the variables that can be referenced in the invariant
        auto availableVars = utils::getVarsForLoopInv(
            invBlock.parentFunction, invBlock.loopHeader);
        // Turn arg-list into sequential collection for consistent ordering
        llvm::SmallVector<llvm::DILocalVariable *> argList(
            availableVars->begin(), availableVars->end());
        // Get insert-pos for wrapper-function
        auto insertPos = wrapperGenerator->getWrapperInsertLoc(invBlock);

        // Generate wrappers for the individual invariants
        for (auto &inv : invBlock.invariants)
        {
            auto wrapperName = wrapperGenerator->getNameForWrapperFunction();
            auto wrapperStr = wrapperGenerator->getWrapperFunctionString(
                argList, inv->expr, wrapperName);
            loopInvWrappers[inv.get()] = std::make_unique<SrcWrapperFunction>(
                wrapperName, wrapperStr, argList, insertPos);
        }
    }

    void SourceSpecTransformer::wrapSpecStatementBlock(
        const MatchedSpecStatementBlock &stmntBlock)
    {
        // Determine the variables that can be referenced in the statement
        auto availableVars = utils::getVarsForStatement(stmntBlock.inst);
        // Turn arg-list into sequential collection for consistent ordering
        llvm::SmallVector<llvm::DILocalVariable *> argList(
            availableVars->begin(), availableVars->end());

        // Get insert-location for wrapper-function
        auto insertPos = wrapperGenerator->getWrapperInsertLoc(stmntBlock);

        // Generate wrapper for the individual statements
        for (auto &stmnt : stmntBlock.statements)
        {
            auto wrapperName = wrapperGenerator->getNameForWrapperFunction();
            auto wrapperStr = wrapperGenerator->getWrapperFunctionString(
                argList, stmnt->expr, wrapperName);
            statementWrappers[stmnt.get()] = std::make_unique<SrcWrapperFunction>(
                wrapperName, wrapperStr, argList, insertPos);
        }
    }

    void SourceSpecTransformer::encodeLoopInvBlock(
        const MatchedLoopInvariantBlock &invBlock)
    {
        const auto numInvs = invBlock.invariants.size();
        if (numInvs == 0)
            return;

        // Determine the variables that can be referenced in the invariant
        // auto availableVars = utils::getVarsForLoopInv(
        //    invBlock.parentFunction, invBlock.loopHeader);

        // Vector of MD-nodes that makes up the tuple with the invariants
        llvm::SmallVector<llvm::Metadata *> invNodes;

        // MD-String that identifies the tuple as a set of loop-invariants
        auto invIDNode = llvm::MDString::get(*irContext,
                                             PALLAS_LOOP_INV_BLOCK);
        invNodes.push_back(invIDNode);

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
        llvm::SmallVector<llvm::Metadata *, 2> mdElems;
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

        // Transform specification statements
        llvm::SmallVector<llvm::Metadata *> specStatementNodes;
        const auto numStatements = contract.statements.size();
        for (size_t i = 0; i < numStatements; ++i)
        {
            auto mdNode = encodeFunctionContractStatement(
                *parentFunction, *contract.statements[i].get());
            specStatementNodes.push_back(mdNode);
        }
        auto contractMDNode = llvm::MDTuple::get(*irContext, specStatementNodes);
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
        llvm::SmallVector<llvm::Metadata *, 2> mdElems;

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

        // Add reference to function
        auto specFuncAsMd = llvm::ValueAsMetadata::get(irWrapperFunction);
        mdElems.push_back(specFuncAsMd);

        // Add references to the metadata-nodes that describe how the arguments
        // of the specification-function relate to the original variables.
        for (auto arg : srcWrapperFun->argList)
            mdElems.push_back(arg);

        return llvm::MDNode::get(*irContext, mdElems);
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

    /*
     * Factory methods
     */
    std::unique_ptr<SourceSpecMatcher> SourceSpecTransformer::createSpecMatcher(
        SpecParser::FileContext *specParseTree, llvm::Module *irModule)
    {
        return std::make_unique<IRBasedSourceSpecMatcher>(
            specParseTree, irModule, &fam, outputConfig.printMatchingInfo);
    }
}