#include <iostream>
#include <cassert>
#include <memory>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

#include "transform/SourceSpecTransformer.h"
#include "utils/IrSpecConstants.h"
#include "utils/SrcVariableCollector.h"
#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"

namespace sourcespec2ir
{
    SourceSpecTransformer::SourceSpecTransformer(std::unique_ptr<SourceSpecContextProvider> specCtxProvider,
                                                 std::unique_ptr<SourceCompilerDriver> compilerDriver, char *irInputPath, char *srcInputPath)
        : specContextProvider(std::move(specCtxProvider))
        , srcCompilerDriver(std::move(compilerDriver))
    {
        // Parse the IR-file
        irContext = std::make_unique<llvm::LLVMContext>();
        irSMDiag = std::make_unique<llvm::SMDiagnostic>();
        irModule = llvm::parseIRFile(irInputPath, *irSMDiag.get(), *irContext.get());

        // Parse the source-file
        srcFileStream = std::make_unique<std::ifstream>();
        srcFileStream->open(srcInputPath);
        srcInStream = std::make_unique<antlr4::ANTLRInputStream>(*srcFileStream);
        srcLexer = std::make_unique<SpecLexer>(srcInStream.get());
        srcTokens = std::make_unique<antlr4::CommonTokenStream>(srcLexer.get());
        srcParser = std::make_unique<SpecParser>(srcTokens.get());
        srcParseTree = srcParser->file();
    }

    void SourceSpecTransformer::transformModule()
    {
        // Traverse the specifications for all functions and transform them into IR
        visit(srcParseTree); 

        // TODO: Invoke LLVm verifier to check the generated IR

        // Dump the generated IR somewhere
        std::cerr << "===============================================" << std::endl;
        irModule->dump();
        std::cerr << "===============================================" << std::endl;
    }

    llvm::Function *SourceSpecTransformer::transformExpression(std::string expr, const llvm::Function& contextFunction, std::string srcFunctionName)
    {
        // Wrap the expression first, into a source-language function and then into a source-compilation unit
        auto srcVars = getSrcVariablesForFunction(contextFunction, true);
        auto wrapperFun = getWrappingFunction(*srcVars, expr);
        auto wrapperCU = assembleSrcCompilationUnit(*wrapperFun);
        // std::cout << "Generated wrapper for " << srcFunctionName << std::endl;
        // std::cout << wrapperFun << std::endl;

        // Compile the specifications into IR & run em2reg optimization
        auto compiledSpecModule = srcCompilerDriver->compileSpecification(wrapperCU, *irContext);
        promoteMemToReg(*compiledSpecModule);

        // Transform the compiled specifications into common format (calls to specification-functions)
        // TODO: Implement !!!

        // Add the generated specifications to the original IR (merge IR files)
        // TODO: Check for name-conflics/ renaming!
        // TODO: Make sure to only link stuff that we actually need and do not include e.g. the spec-lib multiple times
        auto linker = llvm::Linker(*irModule);
        linker.linkInModule(std::move(compiledSpecModule));
        // TODO: Check if it ok that the context of and SMDiagnostics of the compiled module go out of scope here  
        
        // Extract the IR-function that represents the compiled expression
        auto irSpecFunction = getFunctionBySrcName(*irModule, wrapperFun->name);

        return irSpecFunction;
    }

    void SourceSpecTransformer::printSpecifications(char *srcInputPath, char *irInputStream)
    {
        std::cout << "Specification traversal:" << std::endl;
        visit(srcParseTree);

        // Test the IRVariableCollector
        std::cout << "Variable-detection:" << std::endl;
        printAllSrcVars("", irInputStream);

        std::cout.flush();
    }

    void SourceSpecTransformer::promoteMemToReg(llvm::Module &module) {
        llvm::LoopAnalysisManager lam;
        llvm::FunctionAnalysisManager fam;
        llvm::CGSCCAnalysisManager cgam;
        llvm::ModuleAnalysisManager mam;

        // While we do not use the PassBuilder directly, it is required by the ModuleAnalysisManager
        // Causes SegmentationFault otherwise. 
        llvm::PassBuilder passBuilder;

        passBuilder.registerModuleAnalyses(mam);
        passBuilder.registerCGSCCAnalyses(cgam);
        passBuilder.registerFunctionAnalyses(fam);
        passBuilder.registerLoopAnalyses(lam);
        passBuilder.crossRegisterProxies(lam, fam, cgam, mam);

        llvm::ModulePassManager mpm;
        mpm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::PromotePass()));
        mpm.run(module, mam);
    }

    std::any SourceSpecTransformer::visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx)
    {
        // std::cout << "Entering SpecContractBlock: " << std::endl;

        // Get the name of the function to which the contract-block belongs
        auto startLine = ctx->start->getLine();
        auto startCol = ctx->start->getCharPositionInLine();
        auto endLine = ctx->stop->getLine();
        auto endCol = ctx->stop->getCharPositionInLine();
        const auto specFun = specContextProvider->getContractBlockContext(startLine, startCol, endLine, endCol);

        // Get the variables that are accessible in the contract block
        // TODO: Add field to class and query variables from IR once instead of doing this for every statement again.

        // Set source context
        currentSrcContext = specFun;

        auto res = visitChildren(ctx);

        // Reset source context
        currentSrcContext = std::nullopt;

        // std::cout << "Exiting SpecContractBlock: " << std::endl;
        return res;
    }

    std::any SourceSpecTransformer::visitRequiresClause(SpecParser::RequiresClauseContext *ctx)
    {
        // Generate LLVM-IR function for the specification expression.
        auto expr = ctx->specExpression();
        auto exprStr = getExprAsStr(*expr);
        auto irCtxFunction = getFunctionBySrcName(*irModule, currentSrcContext.value());
        auto specFunction = transformExpression(exprStr, *irCtxFunction, currentSrcContext.value());

        // Determine which variables are passed to the specification function:
        // TODO: Implement this

        // Generate the metadata that encodes the requires clause:
        llvm::SmallVector<llvm::Metadata*, 2> mdElems;
        auto specFuncAsMd = llvm::ValueAsMetadata::get(specFunction);
        mdElems.push_back(specFuncAsMd);
        
        // TODO: Pass the variables that make up the arguments of the specification functions
        auto requiresNode = llvm::MDNode::get(*irContext, mdElems);
        
        // Insert the metadata-node into the IR
        irCtxFunction->setMetadata(PALLAS_REQUIRES, requiresNode);

        return 0;
    }

    std::any SourceSpecTransformer::visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx)
    {
        auto expr = ctx->specExpression();
        auto exprStr = getExprAsStr(*expr);
        auto irCtxFunction = getFunctionBySrcName(*irModule, currentSrcContext.value());
        transformExpression(exprStr, *irCtxFunction, currentSrcContext.value());

        // std::cout << "Requires clause: " << getExprAsStr(*expr) << std::endl;
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

    std::unique_ptr<SrcWrapperFunction> SourceSpecTransformer::getWrappingFunction(const llvm::StringMap<std::string> &srcVarsToTypes, const std::string &expr)
    {
        // Name of wrapping function
        auto funName = "PALLAS_SPEC_" + std::to_string(specFunCnt++);

        // Argument-list
        std::string argList = "";
        for (auto it = srcVarsToTypes.begin(), end = srcVarsToTypes.end(); it != end; ++it)
        {
            if (it != srcVarsToTypes.begin())
            {
                argList += " ,";
            }
            const auto varName = it->getKey().str();
            const auto varType = it->getValue();
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
        // TODO: add include-statements form the source-file
        std::string includes = "#include <c_spec_lib.h> \n ";

        return includes + wrapperFunction.stringRepr;
    }

}