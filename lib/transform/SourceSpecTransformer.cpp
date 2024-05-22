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
    SourceSpecTransformer::SourceSpecTransformer(std::unique_ptr<SourceSpecContextProvider> specCtxProvider, char *irInputPath, char *srcInputPath)
        : specContextProvider(std::move(specCtxProvider))
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

        // Transform the compiled specifications into common format (calls to specification-functions)

        // Add the generated specifications to the original IR (merge IR files)

        // Dump the generated IR somewhere
    }


    void SourceSpecTransformer::transformExpression(std::string expr, std::string srcFunctionName)
    {
        // Perform preprocessing of specification expression (e.g. replacement of \result)
        // TODO: Not sure if this is required here

        // Wrap the expression in a source-language function
        auto irFunction = getFunctionBySrcName(*irModule, currentSrcContext.value());
        auto srcVars = getSrcVariablesForFunction(*irFunction, true);
        auto wrapperFun = getWrappingFunction(*srcVars, expr);

        // Assemble the wrapper-function into a compilation-unit of the source-language
        auto wrapperCU = assembleSrcCompilationUnit(wrapperFun);

        // Invoke the compiler-driver to compile the specifications into IR

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
        return wrapperFunction;
    }

}