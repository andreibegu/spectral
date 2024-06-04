#include <iostream>
#include <cassert>
#include <stdexcept>

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

#include "transform/SourceSpecTransformer.h"
#include "utils/IrSpecConstants.h"
#include "utils/IrLookup.h"
#include "utils/SrcLookup.h"
#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"

namespace sourcespec2ir
{
    SourceSpecTransformer::SourceSpecTransformer(std::unique_ptr<SourceSpecContextProvider> specCtxProvider,
                                                 std::unique_ptr<SourceCompilerDriver> compilerDriver,
                                                 std::unique_ptr<SpecUnifier> specUnifier,
                                                 char *srcInputPath)
        : specContextProvider(std::move(specCtxProvider)),
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
        std::string srcPathStr(srcInputPath);
        irModule = srcCompilerDriver->compileSource(srcPathStr, *irContext);

        // Run mem2reg pass on IR module
        promoteMemToReg(*irModule);

        // Parse the source-file to extract the specifications
        srcFileStream = std::make_unique<std::ifstream>();
        srcFileStream->open(srcInputPath);
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
        // TODO: Ensure that context-variables are initialized

        // Traverse the specifications for all functions and transform them into IR
        visit(srcParseTree);

        // Transform the compiled specifications into common format (calls to specification-functions)
        specUnifier->unifySpecifications(irModule.get(), irContext.get());

        // Check wellformedness of module
        assert(!llvm::verifyModule(*irModule, &llvm::errs()));

        // Dump the generated IR somewhere
        std::cerr << "===============================================" << std::endl;
        irModule->dump();
        std::cerr << "===============================================" << std::endl;
    }

    std::unique_ptr<IRWrapperFunction> SourceSpecTransformer::transformExpression(std::string expr, const llvm::Function &contextFunction, std::string srcFunctionName)
    {
        // Collect the variables that can be referenced in the context of the contract.
        auto srcVars = utils::getSrcVariablesForFunction(contextFunction, true);
        // Transfer src-variables to sequential container for explicit ordering
        auto argList = std::make_unique<llvm::SmallVector<llvm::DILocalVariable *>>(srcVars->begin(), srcVars->end());

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

        // Wrap & return function-ptr and arg-list
        return std::make_unique<IRWrapperFunction>(irSpecFunction, std::move(argList));
    }

    void SourceSpecTransformer::promoteMemToReg(llvm::Module &module)
    {
        llvm::ModulePassManager mpm;
        mpm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::PromotePass()));
        mpm.run(module, mam);
    }

    void SourceSpecTransformer::encodeFunctionContractStatement(std::string specExpr, std::string metadataId)
    {
        // TODO: Check for errors (i.e. contextfunction, ... empty)

        // Generate LLVM-IR function for the specification expression.
        auto irCtxFunction = utils::getFunctionBySrcName(*irModule, currentSrcContext.value());
        auto specFunctionWrapper = transformExpression(specExpr, *irCtxFunction, currentSrcContext.value());

        // Generate the metadata that encodes the requires clause:
        llvm::SmallVector<llvm::Metadata *, 2> mdElems;

        auto mdIdentifier = llvm::MDString::get(*irContext, metadataId);
        mdElems.push_back(mdIdentifier);

        auto specFuncAsMd = llvm::ValueAsMetadata::get(specFunctionWrapper->function);
        mdElems.push_back(specFuncAsMd);

        // Add references to the metadata-nodes that describe how the arguments of the
        // specification-function relate to the original variables.
        for (auto arg : *specFunctionWrapper->argListDescr)
        {
            mdElems.push_back(arg);
        }

        auto requiresNode = llvm::MDNode::get(*irContext, mdElems);

        // Add the metadata-node to the list of nodes in the current contract.
        currentSpecMDNodes.value().push_back(requiresNode);
    }

    std::any SourceSpecTransformer::visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx)
    {
        // Get the name of the function to which the contract-block belongs
        auto startLine = ctx->start->getLine();
        auto startCol = ctx->start->getCharPositionInLine();
        auto endLine = ctx->stop->getLine();
        auto endCol = ctx->stop->getCharPositionInLine();
        const auto specFun = specContextProvider->getContractBlockContext(startLine, startCol, endLine, endCol);

        // Get the IR-function that the contract belongs to
        auto irCtxFunction = utils::getFunctionBySrcName(*irModule, specFun);

        // Set context
        currentSrcContext = specFun;
        // TODO: Consider using unique-pointers because the added MD-Nodes are freshly allocated.
        currentSpecMDNodes = std::make_optional<llvm::SmallVector<llvm::MDNode *>>();

        // Transform specification statements
        auto res = visitChildren(ctx);

        // Bundle the metadata-encodings of the spec-statements in a MDTuple add add them to the IR.
        llvm::SmallVector<llvm::Metadata *> specStatementNodes;
        for (auto node : currentSpecMDNodes.value())
        {
            specStatementNodes.push_back(node);
        }
        auto contractMDNode = llvm::MDTuple::get(*irContext, specStatementNodes);
        irCtxFunction->setMetadata(PALLAS_FUN_CONTRACT_BLOCK, contractMDNode);

        // Reset context
        currentSrcContext = std::nullopt;
        currentSpecMDNodes = std::nullopt;

        return res;
    }

    std::any SourceSpecTransformer::visitRequiresClause(SpecParser::RequiresClauseContext *ctx)
    {
        // TODO: Check for errors (i.e. contextfunction, ... empty)

        // Get string representation of specification expression
        auto expr = ctx->specExpression();
        auto exprStr = getExprAsStr(*expr);
        // Encode as metadata-node and add to list of nodes within current function contract.
        encodeFunctionContractStatement(exprStr, PALLAS_REQUIRES);
        return 0;
    }

    std::any SourceSpecTransformer::visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx)
    {
        auto expr = ctx->specExpression();
        auto exprStr = getExprAsStr(*expr);
        // Encode as metadata-node and add to list of nodes within current function contract.
        encodeFunctionContractStatement(exprStr, PALLAS_ENSURES);
        return 0;
    }

    std::any SourceSpecTransformer::visitInvariantClause(SpecParser::InvariantClauseContext *ctx)
    {
        std::cerr << "Loop invariants are currently not supported! " << std::endl;
        assert(false);
        return 0;
    }

    std::string SourceSpecTransformer::getExprAsStr(const SpecParser::SpecExpressionContext &exprCtx)
    {
        auto startToken = exprCtx.start;
        auto stopToken = exprCtx.stop;
        auto charStream = startToken->getTokenSource()->getInputStream();

        antlr4::misc::Interval i(startToken->getStartIndex(), stopToken->getStopIndex());
        return charStream->getText(i);
    }

    std::unique_ptr<SrcWrapperFunction> SourceSpecTransformer::getWrappingFunction(llvm::ArrayRef<llvm::DILocalVariable *> srcVars, const std::string &expr)
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
            const auto varType = utils::getSrcVarType(**it);
            argList += varType + " " + varName;
        }

        auto wFun = "bool " + funName + "(" + argList + ") \n" +
                    "{ \n " +
                    "return " + expr + "; \n" +
                    "} \n";

        return std::make_unique<SrcWrapperFunction>(funName, wFun);
    }

    std::string SourceSpecTransformer::assembleSrcCompilationUnit(const SrcWrapperFunction &wrapperFunction)
    {
        // TODO: add include-statements from the source-file
        std::string includes = "#include <c_spec_lib.h> \n ";

        return includes + wrapperFunction.stringRepr;
    }

}