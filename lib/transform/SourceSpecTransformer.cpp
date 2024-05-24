#include <iostream>
#include <cassert>
#include <memory>

#include <llvm/IRReader/IRReader.h>

#include "transform/SourceSpecTransformer.h"
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

        // Dump the generated IR somewhere
    }

    void SourceSpecTransformer::transformExpression(std::string expr, std::string srcFunctionName)
    {

        // Wrap the expression first, into a source-language function and then into a source-compilation unit
        auto irCtxFunction = getFunctionBySrcName(*irModule, currentSrcContext.value());
        auto srcVars = getSrcVariablesForFunction(*irCtxFunction, true);
        auto wrapperFun = getWrappingFunction(*srcVars, expr);
        auto wrapperCU = assembleSrcCompilationUnit(wrapperFun);

        // Compile the specifications into IR
        auto compiledSpecModule = srcCompilerDriver->compileSpecification(wrapperCU);

        // Extract the IR-function that represents the compiled expression
        // TODO: Remove the hardcoded function-name. 
        // TODO: Also retrieve all functions that are called from the specification.
        //      (I.e. introduce struct wrapper-function that contains, the name, the string-repr, and the function that the specification refers to);
        auto irSpecFunction = getFunctionBySrcName(*compiledSpecModule->llvmModule, "PALLAS_SPEC_" + std::to_string(specFunCnt - 1));

        // Transform the compiled specifications into common format (calls to specification-functions)
        // TODO: Implement

        // Add the generated specifications to the original IR (merge IR files)
        
        // Generate call to the specification-function

        // Do things with the resulting wrapper-function
        // TODO: Store the wrapper functions somewhere (i.e. StringStream)
        std::cout << "Generated wrapper for " << srcFunctionName << std::endl;
        std::cout << wrapperFun << std::endl;
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
        auto expr = ctx->specExpression();
        auto exprStr = getExprAsStr(*expr);
        transformExpression(exprStr, currentSrcContext.value());

        // std::cout << "Requires clause: " << exprStr << std::endl;
        return 0;
    }

    std::any SourceSpecTransformer::visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx)
    {
        auto expr = ctx->specExpression();
        auto exprStr = getExprAsStr(*expr);
        transformExpression(exprStr, currentSrcContext.value());

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

    std::string SourceSpecTransformer::getWrappingFunction(const llvm::StringMap<std::string> &srcVarsToTypes, const std::string &expr)
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
        return wFun;
    }

    std::string SourceSpecTransformer::assembleSrcCompilationUnit(std::string wrapperFunction)
    {
        // TODO: Implement and do smart things
        return "#include <stdbool.h> \n " + wrapperFunction;
    }

}