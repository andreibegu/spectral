#include <cassert>
#include <stdexcept>

#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/LoopSimplify.h>

#include "transform/IRBasedSourceSpecMatcher.h"
#include "transform/SourceSpecTransformer.h"
#include "transform/SpecDeclarationCollector.h"
#include "utils/IrSpecConstants.h"
#include "utils/IrLookup.h"
#include "utils/MetadataUtils.h"
#include "utils/SrcLookup.h"

namespace sourcespec2ir
{
    SourceSpecTransformer::SourceSpecTransformer(
        std::unique_ptr<SourceCompilerDriver> compilerDriver,
        std::unique_ptr<SpecUnifier> specUnifier,
        std::string srcInPath, std::string outPath,
        const utils::OutputConfig &outputConfig)
        : outPath(outPath),
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

        // Compile the source-file into IR
        irModule = srcCompilerDriver->compileSource(srcInPath, *irContext);

        // Simplify loops
        llvm::ModulePassManager mpm;
        mpm.addPass(
            llvm::createModuleToFunctionPassAdaptor(llvm::LoopSimplifyPass()));
        mpm.run(*irModule, mam);

        // Run mem2reg
        promoteMemToReg(*irModule);

        /*
        llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
        llvm::outs() << "PRE-Transformation\n";
        writeToFile(*irModule, "");
        llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
        */

        // Parse the source-file to extract the specifications
        srcFileStream = std::make_unique<std::ifstream>();
        srcFileStream->open(srcInPath);
        srcInStream = std::make_unique<antlr4::ANTLRInputStream>(*srcFileStream);
        srcLexer = std::make_unique<SpecLexer>(srcInStream.get());
        srcTokens = std::make_unique<antlr4::CommonTokenStream>(srcLexer.get());
        srcParser = std::make_unique<SpecParser>(srcTokens.get());
        srcParseTree = srcParser->file();

        if (!srcParseTree)
        {
            throw std::invalid_argument("Unable to parse the provided source file.");
        }
    }

    void SourceSpecTransformer::transformModule()
    {
        // Collect declarations
        SpecDeclarationCollector declCollector(srcParseTree);
        specDeclarations = declCollector.collectDeclarations();

        // Match the specifications to the IR
        auto specMatcher = createSpecMatcher(srcParseTree, irModule.get());
        specMatcher->matchSpecs();

        // Transform the function-contract specifications into IR
        const auto numFContracts = specMatcher->getNumMatchedFuncContracts();
        for (size_t i = 0; i < numFContracts; ++i)
        {
            auto contract = specMatcher->getMatchedFuncContract(i);
            encodeFunctionContractBlock(*contract);
        }

        // Transform loop-invariants into IR
        // TODO: Consider dropping the representation of the invariant-statement
        const auto numLoopInvBlocks = specMatcher->getNumMatchedLoopInv();
        for (size_t i = 0; i < numLoopInvBlocks; ++i)
        {
            auto invBlock = specMatcher->getMatchedLoopInv(i);
            encodeLoopInvBlock(*invBlock);
        }

        // Check wellformedness of module
        assert(!llvm::verifyModule(*irModule, &llvm::errs()));

        /*
        llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
        llvm::outs() << "PRE-Unification\n";
        writeToFile(*irModule, "");
        llvm::outs() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
        */

        // Transform the compiled specifications into common format (calls to specification-functions)
        specUnifier->unifySpecifications(irModule.get(), irContext.get());

        // Check wellformedness of module
        assert(!llvm::verifyModule(*irModule, &llvm::errs()));

        // Dump the generated IR
        writeToFile(*irModule, outPath);
    }

    void SourceSpecTransformer::encodeLoopInvBlock(const MatchedLoopInvariantBlock &invBlock)
    {
        const auto numInvs = invBlock.invariants.size();
        if (numInvs == 0)
            return;

        // Determine the variables that can be referenced in the invariant
        auto availableVars = utils::getVarsForLoopInv(
            invBlock.parentFunction, invBlock.loopHeader);

        // Vector of MD-nodes that makes up the tuple with the invariants
        llvm::SmallVector<llvm::Metadata *> invNodes;

        // MD-String that identifies the tuple as a set of loop-invariants
        auto invIDNode = llvm::MDString::get(*irContext,
                                             PALLAS_LOOP_INV_BLOCK);
        invNodes.push_back(invIDNode);

        // Transform the individual invariants and add them to invNodes
        for (size_t j = 0; j < numInvs; ++j)
        {
            auto *inv = invBlock.invariants[j].get();
            encodeLoopInv(inv->expr, *availableVars.get(), invNodes);
        }

        // Construct MD-tuple and add it to the loop's llvm.loop MD-node
        auto loopMDTuple = invBlock.loopID;
        auto invNode = llvm::MDTuple::get(*irContext, invNodes);
        loopMDTuple->push_back(invNode);
    }

    void SourceSpecTransformer::encodeLoopInv(std::string specExpr,
                                              const std::set<llvm::DILocalVariable *> &vars,
                                              llvm::SmallVector<llvm::Metadata *> &nodes)
    {
        // Transform to IR
        auto irEnc = transformExpression(specExpr, vars);

        // Assemble MD-node to represent invariant-statement
        // The encoding contains a ref to the function that encodes the
        // inv-expression followed by a list of refs to the variables that
        // make up the arguments of that function.
        llvm::SmallVector<llvm::Metadata *, 2> mdElems;
        mdElems.push_back(llvm::ValueAsMetadata::get(irEnc->function));
        for (auto arg : *irEnc->argListDescr)
            mdElems.push_back(arg);
        // Build new node and add it to the vector of the inv-block.
        auto invAsMd = llvm::MDNode::get(*irContext, mdElems);
        nodes.push_back(invAsMd);
    }

    std::unique_ptr<IRWrapperFunction> SourceSpecTransformer::transformExpression(
        std::string expr, const std::set<llvm::DILocalVariable *> &srcVars)
    {
        // Transfer src-variables to sequential container for explicit ordering
        auto argList = std::make_unique<llvm::SmallVector<llvm::DILocalVariable *>>(srcVars.begin(), srcVars.end());

        // Wrap the expression first, into a source-language function and then into a source-compilation unit
        auto srcWrapperFun = getWrappingFunction(*argList, expr);
        auto srcWrapperCU = assembleSrcCompilationUnit(*srcWrapperFun);

        // Compile the specifications into IR & run mem2reg optimization
        auto compiledSpecModule = srcCompilerDriver->compileSpecification(srcWrapperCU, *irContext);
        promoteMemToReg(*compiledSpecModule);

        // Add the generated specifications to the original IR (merge IR files)
        auto linker = llvm::Linker(*irModule);
        linker.linkInModule(std::move(compiledSpecModule));

        // Extract the IR-function that represents the compiled expression
        auto irSpecFunction = utils::getFunctionBySrcName(*irModule, srcWrapperFun->name);

        // Add a metadata-marker to mark it as a generated specification-function
        utils::markGeneratedSpecFunction(*irSpecFunction, *irContext);

        // Wrap & return function-ptr and arg-list
        return std::make_unique<IRWrapperFunction>(irSpecFunction, std::move(argList));
    }

    void SourceSpecTransformer::promoteMemToReg(llvm::Module &module)
    {
        llvm::ModulePassManager mpm;
        mpm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::PromotePass()));
        mpm.run(module, mam);
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
        std::string mdId;
        if (stmnt.type == Requires)
            mdId = PALLAS_REQUIRES;
        else if (stmnt.type == Ensures)
            mdId = PALLAS_ENSURES;
        else
            throw std::logic_error("Unsupported statement-type");

        // Generate LLVM-IR function for the specification expression.
        auto srcVars = utils::getSrcVariablesForFunction(function, true);
        auto specFunctionWrapper = transformExpression(stmnt.expr, *srcVars.get());

        // Generate the metadata that encodes the requires clause:
        llvm::SmallVector<llvm::Metadata *, 2> mdElems;

        auto mdIdentifier = llvm::MDString::get(*irContext, mdId);
        mdElems.push_back(mdIdentifier);

        auto specFuncAsMd = llvm::ValueAsMetadata::get(specFunctionWrapper->function);
        mdElems.push_back(specFuncAsMd);

        // Add references to the metadata-nodes that describe how the arguments of the
        // specification-function relate to the original variables.
        for (auto arg : *specFunctionWrapper->argListDescr)
        {
            mdElems.push_back(arg);
        }

        return llvm::MDNode::get(*irContext, mdElems);
    }

    std::unique_ptr<SrcWrapperFunction> SourceSpecTransformer::getWrappingFunction(
        llvm::ArrayRef<llvm::DILocalVariable *> srcVars, const std::string &expr)
    {

        // Name of wrapping function
        auto funName = "PALLAS_SPEC_" + std::to_string(specFunCnt++);

        // Argument-list
        std::string argList = "";
        for (auto it = srcVars.begin(), end = srcVars.end(); it != end; ++it)
        {
            if (it != srcVars.begin())
            {
                argList += ", ";
            }

            const auto varName = utils::getSrcVarName(**it);
            const auto varType = getSrcReprOfType((*it)->getType());
            argList += varType + " " + varName;
        }

        auto wFun = "bool " + funName + "(" + argList + ") \n" +
                    "{ \n " +
                    "return " + expr + "; \n" +
                    "} \n";

        return std::make_unique<SrcWrapperFunction>(funName, wFun);
    }

    std::string SourceSpecTransformer::assembleSrcCompilationUnit(
        const SrcWrapperFunction &wrapperFunction)
    {
        // TODO: Use a sstream
        // TODO: add include-statements from the source-file
        std::string includes = "#include <c_spec_lib.h> \n ";

        // Add any declarations that were defined in the declaration-block of the specifications.
        for (auto decl : *specDeclarations)
        {
            includes += (decl + "; \n ");
        }

        return includes + wrapperFunction.stringRepr;
    }

    void SourceSpecTransformer::writeToFile(const llvm::Module &module, std::string outPath)
    {
        if (outPath.empty() || outputConfig.printDebugInfo)
        {
            // Print to console if no output-path is given or if debug-info is enabled
            llvm::outs() << "===============================================\n";
            // irModule->dump();
            irModule->print(llvm::outs(), nullptr);
            llvm::outs() << "===============================================\n";
        }

        if (!outPath.empty())
        {
            // Write the IR-representation to the given file.
            std::error_code eCode;
            llvm::raw_fd_ostream outStream(outPath, eCode, llvm::sys::fs::CD_CreateAlways);
            if (!eCode)
            {
                // Use IRPrintingPasses alternatively
                module.print(outStream, nullptr, false, true);
            }
            else
            {
                throw std::runtime_error("Failed to write output to " +
                                         std::string(outPath) + ". Error code: " + eCode.message());
            }
            outStream.close();
        }
    }

    /**
     * TODO: Clean this up, and make it more source-language independent!!!
     * TODO: Support more Tags
     */
    std::string SourceSpecTransformer::getSrcReprOfType(const llvm::DIType *type)
    {
        // Check if type is a pointer:
        if (auto *dType = llvm::dyn_cast<llvm::DIDerivedType>(type))
        {
            auto tag = dType->getTag();
            auto baseType = dType->getBaseType();

            if (tag == llvm::dwarf::DW_TAG_pointer_type)
            {
                // Handle Pointer-type
                // Recursively unwrap the pointer
                auto baseRepr = getSrcReprOfType(baseType);
                return baseRepr + "*";
            }
            else if (tag == llvm::dwarf::DW_TAG_const_type)
            {
                // Handle const-types
                return getSrcReprOfType(baseType) + " const ";
            }
            else
            {
                throw std::logic_error("Encountered an unsupported type-tag: " + dType->getTag());
            }
        }

        // Assume that the DIType is a plain DIType and just get the name
        auto typeName = type->getName().str();
        if (typeName.empty())
        {
            throw std::logic_error("Failed to generate string-representation for type.");
        }
        return typeName;
    }

    /*
     * Factory methods
     */
    std::unique_ptr<SourceSpecMatcher> SourceSpecTransformer::createSpecMatcher(
        SpecParser::FileContext *specParseTree, llvm::Module *irModule)
    {
        return std::make_unique<IRBasedSourceSpecMatcher>(specParseTree, irModule,
                                                          &fam, outputConfig.printMatchingInfo);
    }
}