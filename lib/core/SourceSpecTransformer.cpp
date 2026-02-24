#include <map>
#include <stdexcept>
#include <vector>

#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>
#include <llvm/Transforms/Utils/LoopSimplify.h>
#include <llvm/Transforms/Scalar/LoopRotation.h>

#include "IRBasedSourceSpecMatcher.h"
#include "SourceSpecTransformer.h"
#include "SpecDeclarationCollector.h"
#include "SpecEncoder.h"
#include "SpecPredicateCollector.h"
#include "FileUtils.h"
#include "IrSpecConstants.h"
#include "IRUtils.h"
#include "MetadataUtils.h"
#include "PallasException.h"
#include "SrcLookup.h"
#include "WrapperLocCollector.h"
#include "passes/LoopIDPass.h"
#include "wrapper/SpecParserWrapper.h"
#include "wrapper/MatchedWrapperFunctions.h"

namespace sourcespec2ir
{
    SourceSpecTransformer::SourceSpecTransformer(
        std::unique_ptr<SourceCompilerDriver> compilerDriver,
        std::unique_ptr<SpecUnifier> specUnifier,
        std::string srcInPath, std::string outPath,
        llvm::SmallVector<std::string> extContractPaths,
        const utils::OutputConfig &outputConfig,
        const std::string &workingDir,
        bool runMem2Reg)
        : srcInPath(srcInPath),
          outPath(outPath),
          extContractPaths(extContractPaths),
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

        // Simplify loops & add the !llvm.loop-node to all of them
        llvm::ModulePassManager mpm;
        mpm.addPass(
            llvm::createModuleToFunctionPassAdaptor(llvm::LoopSimplifyPass()));
        mpm.addPass(llvm::createModuleToFunctionPassAdaptor(
            llvm::createFunctionToLoopPassAdaptor(LoopIDPass())));
        mpm.run(*irModule, mam);

        // Run mem2reg only if cl-option was set.
        if (runMem2Reg)
            promoteMemToReg(*irModule);

        // Parse the source-file to extract the specifications
        auto inFileChecksum = utils::getChecksumForFile(srcInPath);
        auto parser = std::make_unique<SpecParserWrapper>(srcInPath);
        auto srcParseTree = parser->getSrcParseTree();
        if (!srcParseTree)
            throw utils::PallasException("Unable to parse the source file.");

        // Collect declarations
        SpecDeclarationCollector declCollector(
            srcParseTree, srcInPath, inFileChecksum);
        auto specDeclarations = declCollector.collectDeclarations();

        // Collect predicate definitions
        auto specPredicates =
            SpecPredicateCollector(*srcParseTree, srcInPath, inFileChecksum,
                                   outputConfig.printMatchingInfo)
                .collect();

        // Collect location-markers:
        WrapperLocCollector wLocCollector(*srcParseTree, srcInPath,
                                          inFileChecksum);
        auto wLocMarkers = wLocCollector.collectLocationMarkers();

        // Match the specifications to the IR
        // (This also collects the ghost functions)
        auto specMatcher = createSpecMatcher(srcParseTree, irModule.get(),
                                             srcInPath, false);
        auto matchedSpecs = specMatcher->matchSpecs();

        // Release the parser and input-streams
        parser.reset();

        // TODO: generalize so that multiple src-files are supported and normal
        //          specs are only matched within the same file!
        for (auto contractPath : extContractPaths)
            matchedSpecs->merge(matchExternalContract(contractPath));

        if (matchedSpecs->hasDuplicateMappings())
            throw utils::PallasException(
                "Multiple specifications were matched to the same target");

        // Ensure that specifications about ghost variables are wellformed
        checkGhostArgsWellformed(*matchedSpecs);
        checkGhostAssignsWellformed(*matchedSpecs);

        // Generate the wrapper-functions:
        wrapperGenerator = createWrapperGenerator(
            *specDeclarations, *wLocMarkers);
        matchedWrappers = wrapperGenerator->generateWrapperFunctions(*matchedSpecs);
        auto wrappers = matchedWrappers->getWrappers();

        // Insert wrappers into source
        // TODO: Either move this into the WrapperGenerator, or into a new class
        const auto srcWrapperPath = getPathForSourceWithWrappers();
        auto ghostMethods = matchedSpecs->getGhostMethods();
        wrapperGenerator->buildSrcWithWrappers(srcInPath, srcWrapperPath,
                                               *wrappers, *specPredicates,
                                               ghostMethods);

        // Compile source
        auto moduleWithWrappers = srcCompilerDriver->compileSource(
            srcWrapperPath, *irContext);

        // Turn all duplicate global values into declarations to avoid conflicts
        // while linking.
        removeDuplicateDefinitions(*irModule, *moduleWithWrappers);

        // Apply mem2reg to new module
        promoteMemToReg(*moduleWithWrappers);

        // Remove compiler-generated arguments from wrapper-functions
        // that belong to an external function.
        auto extWrappers = matchedWrappers->getExternalWrappers();
        pruneWrapperArgs(*moduleWithWrappers, extWrappers);

        // Add all generated wrapper to the llvm.used-list of the module
        // to ensure that they are copied over during linking.
        markWrapperFunctionsAsUsed(*moduleWithWrappers, *wrappers);
        // Mark specification functions
        markPredicateDefs(*moduleWithWrappers, *specPredicates);
        markGhostMethods(*moduleWithWrappers, ghostMethods);

        auto linker = llvm::Linker(*irModule);
        linker.linkInModule(std::move(moduleWithWrappers),
                            llvm::Linker::Flags::LinkOnlyNeeded);

        if (outputConfig.printDebugInfo)
            verifyModule(*irModule, "After linking in wrappers");

        // Update the debug-locations in the generated wrapper-functions
        // to match the locations in the original source-file.
        rewriteSpecLocations(*matchedSpecs, *specPredicates);

        if (outputConfig.printDebugInfo)
            verifyModule(*irModule, "After location rewrite");

        // Extend wrappers of ghost contracts with args of the corresponding
        // ghost function.
        // This must happen after linking and location rewriting,
        // otherwise the DIVars are wrong
        addArgsToGhostContracts(*matchedSpecs, *irModule);

        if (outputConfig.printDebugInfo)
        {
            llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++\n";
            llvm::outs() << "PRE-Unification\n";
            writeToFile(*irModule, "");
            llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++\n";
        }

        // Encode specifications into metadata
        SpecEncoder encoder(*irContext, *irModule, *matchedWrappers);
        encoder.encodeSpecifications(*matchedSpecs);

        // Check wellformedness of module
        if (outputConfig.printDebugInfo)
            verifyModule(*irModule, "After transformation");

        // Transform the compiled specifications into common format
        // (calls to specification-functions)
        specUnifier->unifySpecifications(irModule.get(), irContext.get());

        // Check wellformedness of module
        verifyModule(*irModule, "After unification");

        // Dump the generated IR
        writeToFile(*irModule, outPath);
    }

    std::unique_ptr<MatchedSpecifications>
    SourceSpecTransformer::matchExternalContract(const std::string &contractPath)
    {
        auto parser = std::make_unique<SpecParserWrapper>(contractPath);
        auto contractParseTree = parser->getSrcParseTree();
        if (!contractParseTree)
            throw utils::PallasException(
                "Unable to parse external contract: " + contractPath);
        // Match external contracts
        auto matcher = createSpecMatcher(
            contractParseTree, irModule.get(), contractPath, true);
        auto extContracts = matcher->matchSpecs();
        return extContracts;
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

    llvm::DICompileUnit *SourceSpecTransformer::getCompileUnit(
        const std::string &path)
    {
        // TODO: Find a better way to set the language
        unsigned int lang = 0;
        if (path.ends_with(".h") || path.ends_with(".c"))
        {
            lang = llvm::dwarf::DW_LANG_C;
        }
        else if (path.ends_with(".hpp") || path.ends_with(".cpp"))
        {
            lang = llvm::dwarf::DW_LANG_C_plus_plus;
        }
        else if (path.ends_with(".swift"))
        {
            lang = llvm::dwarf::DW_LANG_Swift;
        }

        llvm::DIBuilder builder(*irModule);
        auto *file = builder.createFile(path, "");
        auto *cu = builder.createCompileUnit(
            lang,       // lang
            file,       // file
            "spectral", // producer
            false,      // isOptimized
            "",         // flags
            0           // runtime-version
        );
        builder.finalize();
        return cu;
    }

    void SourceSpecTransformer::rewriteSpecLocations(
        const MatchedSpecifications &matchedSpecs,
        llvm::ArrayRef<SpecPredicate> predDefs)
    {

        // Rewrite locations in ghost definitions
        for (auto &g : matchedSpecs.ghostMethods)
        {
            auto f = utils::getFunctionBySrcName(*irModule, g->name);
            if (f == nullptr)
                throw utils::PallasException("Failed to get ghost");
            auto path = utils::makeAbsPath(g->srcLoc.getFile());
            llvm::DICompileUnit *cu = getCompileUnit(path);
            rewriteSpecLocations(*f, g->defLoc, std::make_pair(0, 0),
                                 cu, cu->getFile(), cu->getFile());
        }

        // Rewrite locations in predicate definitions
        for (auto &p : predDefs)
        {
            auto irFunc = utils::getFunctionBySrcName(*irModule, p.name);
            if (irFunc == nullptr)
                throw utils::PallasException(
                    "Failed to get IR-function for predicate");
            auto path = utils::makeAbsPath(p.srcLoc.getFile());
            llvm::DICompileUnit *cu = getCompileUnit(path);
            rewriteSpecLocations(
                *irFunc,
                p.exprLoc,
                wrapperGenerator->getOffsetToExprStart(p),
                cu,
                cu->getFile(),
                cu->getFile());
        }

        // Handle function-contracts
        for (auto &contr : matchedSpecs.matchedFContracts)
        {
            auto contrPath = utils::makeAbsPath(contr->srcLoc.getFile());
            bool externalFile =
                contrPath != utils::makeAbsPath(getSrcInPath());

            llvm::DICompileUnit *cu = nullptr;
            llvm::DIFile *scopeFile = nullptr;
            llvm::DIScope *scope = nullptr;
            if (externalFile || contr->type == External)
            {
                // For contracts in external files, point scopes to that file
                cu = getCompileUnit(contrPath);
                scopeFile = cu->getFile();
                scope = cu->getFile();
            }
            else
            {
                // Otherwise, point to scope, ... of the parent function.
                cu = getCUForFunc(contr->function);
                scopeFile = getFileForFunc(contr->function);
                scope = getScopeForFunc(contr->function);
            }
            for (auto &stmnt : contr->statements)
                rewriteWrapperLocations(*matchedWrappers->fContractWrappers[stmnt.get()],
                                        stmnt->exprLoc, cu, scopeFile, scope);
        }
        // Handle ghost contracts
        for (auto &c : matchedSpecs.matchedGhostContracts)
        {
            auto f = utils::getFunctionBySrcName(*irModule, c->method->name);
            if (f == nullptr)
                throw utils::PallasException("Failed to get ghost");

            auto contrPath = utils::makeAbsPath(c->srcLoc.getFile());
            for (auto &stmnt : c->statements)
                rewriteWrapperLocations(
                    *matchedWrappers->ghostContractWrappers[stmnt.get()],
                    stmnt->exprLoc, f);
        }
        // Handle loop-invariants
        for (auto &invBlock : matchedSpecs.matchedLoopInvariants)
        {
            for (auto &inv : invBlock->invariants)
                rewriteWrapperLocations(*matchedWrappers->loopInvWrappers[inv.get()],
                                        inv->exprLoc, invBlock->parentFunction);
        }
        // Handle specification statements
        for (auto &stmntBlock : matchedSpecs.matchedStmntBlocks)
        {
            auto parentFunc = stmntBlock->inst->getParent()->getParent();
            for (auto &stmnt : stmntBlock->statements)
                rewriteWrapperLocations(
                    *matchedWrappers->statementWrappers[stmnt.get()],
                    stmnt->exprLoc, parentFunc);
        }
        // Handle given-arg bindings
        for (auto &bindingBlock : matchedSpecs.matchedGivenBindings)
        {
            auto parentFunc = bindingBlock->call->getParent()->getParent();
            for (auto &b : bindingBlock->bindings)
                rewriteWrapperLocations(
                    *matchedWrappers->givenBindingWrappers[&b], b.exprLoc, parentFunc);
        }
    }

    namespace
    {
        void throwDuplicateGhostArgDefName(
            const std::string &argName, const llvm::Function &f)
        {
            std::stringstream s;
            s << "Duplicate name of ghost argument '"
              << argName << "' in function '"
              << utils::getSrcNameOfFunction(f) << "'";
            throw utils::PallasException(s.str());
        }

        void throwBindingNoContract(const MatchedGhostArgBindingBlock &bindings,
                                    const llvm::Function &f)
        {
            std::stringstream s;
            s << "Ghost binding (" << bindings.srcLoc.asShortString() << ") "
              << "attached to function without ghost arguments: "
              << utils::getSrcNameOfFunction(f);
            throw utils::PallasException(s.str());
        }
    };

    void SourceSpecTransformer::checkGhostArgsWellformed(
        const MatchedSpecifications &matchedSpec) const
    {
        // Check that names of ghost args do not conflict
        for (const auto &contr : matchedSpec.matchedFContracts)
        {
            llvm::StringSet argNames;
            auto parentArgs = getDIVarsOfArgs(*contr->function);
            for (auto *v : parentArgs)
                argNames.insert(utils::getSrcVarName(*v));
            for (auto &gArg : contr->givenArgs)
            {
                if (argNames.contains(gArg.argName))
                    throwDuplicateGhostArgDefName(gArg.argName, *contr->function);
                argNames.insert(gArg.argName);
            }
            for (auto &yArg : contr->yieldsArgs)
            {
                if (argNames.contains(yArg.argName))
                    throwDuplicateGhostArgDefName(yArg.argName, *contr->function);
                argNames.insert(yArg.argName);
            }
        }

        // Check that given-bindings mention all given-args
        for (const auto &gBindings : matchedSpec.matchedGivenBindings)
        {
            // Check that the called function has a contract
            auto *calledFunc = gBindings->call->getCalledFunction();
            auto *calleeContr = matchedSpec.getContractForFunc(*calledFunc);
            if (calleeContr == nullptr)
                throwBindingNoContract(*gBindings, *calledFunc);
            // Check that the number of given args matches
            if (calleeContr->givenArgs.size() != gBindings->bindings.size())
            {
                std::stringstream s;
                s << "Number of given-args does not match between call "
                  << "and contract (" << gBindings->srcLoc.asShortString() << ")";
                throw utils::PallasException(s.str());
            }

            // Check all args in the binding-block
            llvm::StringSet gArgNames;
            for (const auto &b : gBindings->bindings)
            {
                // Check for duplicates
                if (gArgNames.contains(b.ghostArg))
                {
                    std::stringstream s;
                    s << "Given argument occurs more than once:"
                      << b.ghostArg << " at " << b.srcLoc.asShortString();
                    throw utils::PallasException(s.str());
                }
                gArgNames.insert(b.ghostArg);
                // Check if defined in contract
                if (calleeContr->getGivenArg(b.ghostArg) == nullptr)
                {
                    std::stringstream s;
                    s << "Argument " << b.ghostArg
                      << " (" << b.srcLoc.asShortString() << ") "
                      << " is not defined in contract of called function.";
                    throw utils::PallasException(s.str());
                }
            }
        }

        // Check yields-binding blocks
        for (const auto &yBindings : matchedSpec.matchedYieldsBindings)
        {
            // Check that the called function has a contract
            auto *calledFunc = yBindings->call->getCalledFunction();
            auto *calleeContr = matchedSpec.getContractForFunc(*calledFunc);
            if (calleeContr == nullptr)
                throwBindingNoContract(*yBindings, *calledFunc);

            // Check all args in the binding-block
            llvm::StringSet yArgNames;
            for (const auto &b : yBindings->bindings)
            {
                // Check for duplicates
                if (yArgNames.contains(b.ghostArg))
                {
                    std::stringstream s;
                    s << "Yields argument occurs more than once:"
                      << b.ghostArg << " at " << b.srcLoc.asShortString();
                    throw utils::PallasException(s.str());
                }
                yArgNames.insert(b.ghostArg);
                // Check if defined in contract
                if (calleeContr->getYieldsArg(b.ghostArg) == nullptr)
                {
                    std::stringstream s;
                    s << "Argument " << b.ghostArg
                      << " (" << b.srcLoc.asShortString() << ") "
                      << " is not defined in contract of called function.";
                    throw utils::PallasException(s.str());
                }
            }
        }
    }

    void SourceSpecTransformer::checkGhostAssignsWellformed(
        const MatchedSpecifications &matchedSpec) const
    {
        for (const auto &stmntBlock : matchedSpec.matchedStmntBlocks)
        {
            auto *parentFunc = stmntBlock->inst->getParent()->getParent();
            auto *parentContr = matchedSpec.getContractForFunc(*parentFunc);
            for (const auto &stmnt : stmntBlock->statements)
            {
                if (stmnt->type != GhostAssign)
                    continue;

                // Check that the parent function has a contract
                if (parentContr == nullptr)
                {
                    std::stringstream s;
                    s << "Ghost assignment ("
                      << stmntBlock->srcLoc.asShortString()
                      << ") in function without contract ("
                      << utils::getSrcNameOfFunction(*parentFunc) << ")";
                    throw utils::PallasException(s.str());
                }

                // Check that the ghost variable is defined in the function
                if (!stmnt->ghostVarName.has_value())
                {
                    std::stringstream s;
                    s << "Ghost assignment at "
                      << stmnt->srcLoc.asShortString()
                      << " does not have an associated ghost variable! ";
                    throw utils::PallasException(s.str());
                }
                if (parentContr->getYieldsArg(stmnt->ghostVarName.value()) == nullptr)
                {
                    std::stringstream s;
                    s << "Ghost variable " << stmnt->ghostVarName.value()
                      << " is not defined or assignable in assignment at "
                      << stmnt->srcLoc.asShortString();
                    throw utils::PallasException(s.str());
                }
            }
        }
    }

    void SourceSpecTransformer::rewriteWrapperLocations(
        const SrcWrapperFunction &wFun, const SrcLocation &exprStartLoc,
        llvm::Function *parentFunc)
    {
        auto cu = getCUForFunc(parentFunc);
        auto scopeFile = getFileForFunc(parentFunc);
        auto scope = getScopeForFunc(parentFunc);
        rewriteWrapperLocations(wFun, exprStartLoc, cu, scopeFile, scope);
    }

    void SourceSpecTransformer::rewriteWrapperLocations(
        const SrcWrapperFunction &wFun, const SrcLocation &exprStartLoc,
        llvm::DICompileUnit *cu, llvm::DIFile *scopeFile,
        llvm::DIScope *scope)
    {
        // Get the wrapper-function
        auto irWrapperF = utils::getFunctionBySrcName(*irModule, wFun.name);
        if (irWrapperF == nullptr)
            throw utils::PallasException("Failed to get wrapper-function by name");
        auto offset = wrapperGenerator->getOffsetToExprStart(wFun);
        rewriteSpecLocations(*irWrapperF, exprStartLoc, offset, cu,
                             scopeFile, scope);
    }

    void SourceSpecTransformer::rewriteSpecLocations(
        llvm::Function &irFunc, const SrcLocation &exprStartLoc,
        std::pair<size_t, size_t> srcOffset,
        llvm::DICompileUnit *cu, llvm::DIFile *scopeFile,
        llvm::DIScope *scope)
    {
        auto *oldSProg = irFunc.getSubprogram();
        auto oldFLine = oldSProg->getLine(); // Needed to calculate new positions for instructions
        llvm::DIBuilder diBuilder(*irModule, true, cu);

        // Update the location of the functions DISubprogram
        // The line-info cannot be modified manually, so we replace the full
        // DISubprogram of the function.
        auto newSProg = diBuilder.createFunction(
            scope, // New scope
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
        irFunc.setSubprogram(newSProg);

        // Update the dbg-locations of the instructions within the function
        auto [lineOffset, colOffset] = srcOffset;
        for (auto it = inst_begin(irFunc), end = inst_end(irFunc); it != end; ++it)
        {
            if (!it->getDebugLoc())
                continue;

            // If the DILocation is inlined, we need to consider the
            // location that is pointed to by inlinedAt.
            auto inlinedLoc = it->getDebugLoc().getInlinedAt();
            auto oldLine = inlinedLoc != nullptr ? inlinedLoc->getLine() : it->getDebugLoc().getLine();
            auto oldCol = inlinedLoc != nullptr ? inlinedLoc->getColumn() : it->getDebugLoc().getCol();
            auto oldScope = inlinedLoc != nullptr ? inlinedLoc->getScope() : it->getDebugLoc().getScope();

            // Check if the instruction points to the header of the wrapper-function
            const bool pointsToHeader = oldLine < oldFLine + lineOffset;
            // Check if the new-column would point to an invalid location
            // I.e. is the instr on the first line of the expr and
            // is the col smaller than the offset
            const bool newColInvalid =
                oldLine - oldFLine - lineOffset == 0 &&
                oldCol < colOffset;

            //  Replace references to old DISubprogram in the scope of the location
            auto newScope = oldScope;
            if (auto lScope = llvm::dyn_cast<llvm::DIScope>(newScope))
                newScope = replaceSubprogInScope(lScope, oldSProg, newSProg, diBuilder);

            // If the new location would be invalid, set line- and column to 0,
            // otherwise calculate new location.
            // Dropping the location altogether does not work for debug-intrinsics
            // as it causes the verify-pass to fail.
            unsigned int newLine = 0;
            unsigned int newCol = 0;
            if (!pointsToHeader && !newColInvalid)
            {
                newLine = exprStartLoc.line + oldLine - oldFLine - lineOffset;
                newCol = oldCol;
                // If the instruction is on the first-line, we need to add
                // the additional column-offset.
                if (newLine == exprStartLoc.line)
                {
                    newCol += exprStartLoc.column;
                    newCol -= colOffset;
                }
            }

            // Construct new debug-location:
            if (it->getDebugLoc().getInlinedAt() != nullptr)
            {
                // DILocation was inlined
                if (inlinedLoc->getInlinedAt() != nullptr)
                    throw utils::PallasException(
                        "Location-rewrite does not support nested inlining.");

                // Build new version of the inlinedAt-location:
                auto iLoc = llvm::DILocation::get(
                    *irContext,
                    newLine,
                    newCol,
                    newScope,
                    inlinedLoc->getInlinedAt(),
                    inlinedLoc->isImplicitCode());
                // Build new debug-location
                auto newLoc = llvm::DILocation::get(
                    *irContext,
                    it->getDebugLoc().getLine(),
                    it->getDebugLoc().getCol(),
                    it->getDebugLoc().getScope(),
                    iLoc,
                    it->getDebugLoc().isImplicitCode());
                it->setDebugLoc(llvm::DebugLoc(newLoc));
            }
            else
            {
                // DILocation was not inlined
                auto isImplicit = it->getDebugLoc().isImplicitCode();
                auto newDebugLoc = llvm::DILocation::get(
                    *irContext, newLine, newCol, newScope, nullptr, isImplicit);
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

    llvm::DIScope *SourceSpecTransformer::replaceSubprogInScope(
        llvm::DIScope *scope, const llvm::DISubprogram *oldSubprog,
        llvm::DISubprogram *newSubprog, llvm::DIBuilder &diBuilder)
    {
        // Only consider cases, where the passed scope is a local scope.
        auto oldScope = llvm::dyn_cast_if_present<llvm::DILocalScope>(scope);
        if (oldScope == nullptr)
            return scope;

        // If the passed scope is the old subprogram, return new subprogram
        if (oldScope == oldSubprog)
            return newSubprog;

        // Recursively handle the parent-scope
        auto oldParentScope = oldScope->getScope();
        auto newParentScope = replaceSubprogInScope(
            oldParentScope, oldSubprog, newSubprog, diBuilder);

        // If no changes to the parent-scope were made, return
        if (newParentScope == oldParentScope)
            return oldScope;

        // Return a copy of the old-scope where the parent-scope has been replaced
        if (auto sProg = llvm::dyn_cast<llvm::DISubprogram>(oldScope))
        {
            return diBuilder.createFunction(
                newParentScope, // New scope
                sProg->getName(),
                sProg->getLinkageName(),
                sProg->getFile(),
                sProg->getLine(),
                sProg->getType(),
                sProg->getScopeLine(),
                sProg->getFlags(),
                sProg->getSPFlags(),
                sProg->getTemplateParams(),
                sProg->getDeclaration(),
                sProg->getThrownTypes(),
                sProg->getAnnotations(),
                sProg->getTargetFuncName());
        }
        else if (auto lBlock = llvm::dyn_cast<llvm::DILexicalBlock>(oldScope))
        {
            return diBuilder.createLexicalBlock(
                newParentScope,
                lBlock->getFile(),
                lBlock->getLine(),
                lBlock->getColumn());
        }
        else if (auto lbFile = llvm::dyn_cast<llvm::DILexicalBlockFile>(oldScope))
        {
            return diBuilder.createLexicalBlockFile(
                newParentScope,
                lbFile->getFile(),
                lbFile->getDiscriminator());
        }

        throw utils::PallasException("Failed to rewrite scopes in debug-info.");
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

    bool SourceSpecTransformer::isCompilerGeneratedArg(const llvm::Argument &arg)
    {
        // Check for self-arg of swift
        if (arg.hasSwiftSelfAttr())
            return true;
        // Check if the dbg-info has the DIFlagObjectPointer-attribute
        auto argIdx = arg.getArgNo();
        auto *wSProg = arg.getParent()->getSubprogram();
        auto argDIType = wSProg->getType()->getTypeArray()[argIdx + 1];
        if (argDIType->isObjectPointer())
            return true;
        return false;
    }

    void SourceSpecTransformer::pruneWrapperArgs(
        llvm::Module &irModule,
        llvm::ArrayRef<SrcWrapperFunction *> wrappers)
    {
        for (auto *wrapper : wrappers)
        {
            auto wFunc = utils::getFunctionBySrcName(irModule, wrapper->name);
            if (wFunc == nullptr)
                throw utils::PallasException("Failed to get wrapper-function");

            // Collect compiler-generated arguments
            llvm::SmallSet<llvm::Argument *, 2> genArgs;
            llvm::SmallSet<unsigned, 2> argIdxs;
            for (auto &arg : wFunc->args())
            {
                if (isCompilerGeneratedArg(arg))
                {
                    genArgs.insert(&arg);
                    argIdxs.insert(arg.getArgNo() + 1);
                }
            }
            if (genArgs.empty())
                return;

            // TODO: Check that compiler-generated args are only at front and back

            // Collect & remove dbg-intrinscs that refer to generated args
            llvm::SmallVector<llvm::DbgVariableIntrinsic *, 2> intrToDelete;
            for (auto i = inst_begin(wFunc), end = inst_end(wFunc); i != end; ++i)
            {
                if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*i))
                {
                    auto *var = intr->getVariable();
                    if (var->isParameter() && argIdxs.contains(var->getArg()))
                        intrToDelete.push_back(intr);
                }
            }
            while (!intrToDelete.empty())
                intrToDelete.pop_back_val()->eraseFromParent();

            // Check that compiler-generated args are no longer used in the function
            for (auto *arg : genArgs)
                if (!arg->use_empty())
                    throw utils::PallasException(
                        "Failed to prune wrapper function (use of generated arg)");

            // Make sure that the wrapper is not referenced in the modules used-list
            llvm::removeFromUsedLists(*wFunc->getParent(),
                                      [wFunc](llvm::Constant *C)
                                      { return C == wFunc; });

            // Remove compiler-generated args from function
            utils::removeArgsFromWrapper(wFunc, genArgs);
        }
    }

    void SourceSpecTransformer::addArgsToGhostContracts(
        const MatchedSpecifications &matchedSpecs, llvm::Module &irModule)
    {
        for (auto &gContr : matchedSpecs.matchedGhostContracts)
        {
            auto fName = gContr->method->name;
            auto *irGhostF = utils::getFunctionBySrcName(irModule, fName);
            if (irGhostF == nullptr)
                throw utils::PallasException(
                    "Failed to find ghost function " + fName);
            // Collect DIVariables that point to args of the ghost function
            auto diArgs = getDIVarsOfArgs(*irGhostF);

            // Add DIVars to wrappers
            for (auto &stmnt : gContr->statements)
            {
                if (!matchedWrappers->ghostContractWrappers.contains(stmnt.get()))
                    throw utils::PallasException(
                        "Failed to find wrapper for ghost contract statement");
                auto &wrapper = *matchedWrappers->ghostContractWrappers.at(stmnt.get());
                wrapper.argList.append(diArgs.begin(), diArgs.end());
            }
        }
    }

    void SourceSpecTransformer::markWrapperFunctionsAsUsed(
        llvm::Module &module, llvm::ArrayRef<SrcWrapperFunction *> wrappers)
    {
        llvm::SmallVector<llvm::GlobalValue *> irWrappers;
        for (auto &wrapper : wrappers)
        {
            auto wrapperF = utils::getFunctionBySrcName(module, wrapper->name);
            if (wrapperF == nullptr)
                throw utils::PallasException("Failed to get wrapper-function");
            irWrappers.push_back(wrapperF);
        }
        llvm::appendToUsed(module, irWrappers);
    }

    void SourceSpecTransformer::markPredicateDefs(
        llvm::Module &module, llvm::ArrayRef<SpecPredicate> preds)
    {
        llvm::SmallVector<llvm::GlobalValue *> irFunctions;
        for (auto &p : preds)
        {
            auto f = utils::getFunctionBySrcName(module, p.name);
            if (f == nullptr)
                throw utils::PallasException("Failed to get predicate");
            utils::markPredicateDef(*f, module.getContext(), p.isInline);
            irFunctions.push_back(f);
        }
        llvm::appendToUsed(module, irFunctions);
    }

    void SourceSpecTransformer::markGhostMethods(
        llvm::Module &module, llvm::ArrayRef<GhostMethod *> ghostMethods)
    {
        llvm::SmallVector<llvm::GlobalValue *> irGhosts;
        for (auto &ghost : ghostMethods)
        {
            auto f = utils::getFunctionBySrcName(module, ghost->name);
            if (f == nullptr)
                throw utils::PallasException("Failed to get ghost");
            utils::markGhostMethod(*f, module.getContext());
            irGhosts.push_back(f);
        }
        llvm::appendToUsed(module, irGhosts);
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

    llvm::DIScope *SourceSpecTransformer::getScopeForFunc(llvm::Function *f)
    {
        if (f == nullptr)
            throw utils::PallasException("Passed function is nullptr.");
        if (f->getSubprogram() == nullptr)
            throw utils::PallasException("Encountered a function without a subprogram");
        auto *scope = f->getSubprogram()->getScope();
        return scope;
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

    llvm::SmallSet<llvm::DILocalVariable *, 8> 
    SourceSpecTransformer::getDIVarsOfArgs(const llvm::Function &f) const
    {
        llvm::SmallSet<llvm::DILocalVariable *, 8> diArgs;
        for (auto it = llvm::inst_begin(f), end = llvm::inst_end(f); it != end; ++it)
        {
            if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*it))
            {
                auto *v = intr->getVariable();
                if (v->getArg() != 0)
                    diArgs.insert(v);
            }
        }
        return diArgs;
    }

    /*
     * Factory methods
     */
    std::unique_ptr<SourceSpecMatcher> SourceSpecTransformer::createSpecMatcher(
        SpecParser::FileContext *specParseTree, llvm::Module *irModule,
        std::string srcFilePath, bool matchExternalOnly)
    {
        return std::make_unique<IRBasedSourceSpecMatcher>(
            specParseTree, srcFilePath, irModule, &fam,
            outputConfig.printMatchingInfo, matchExternalOnly, 
            createSyntaxResolver());
    }

    std::string SourceSpecTransformer::getSrcInPath()
    {
        return srcInPath;
    }

    std::unique_ptr<SourceSyntaxResolver> 
    SourceSpecTransformer::createSyntaxResolver() {
        return std::make_unique<SourceSyntaxResolver>();
    }
}
