#include <iostream>

#include "transform/SourceSpecTransformer.h"
#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"

namespace sourcespec2ir
{

    void SourceSpecTransformer::printSpecifications(char *inputPath)
    {
        std::ifstream srcFileStream;
        srcFileStream.open(inputPath);

        antlr4::ANTLRInputStream inStream(srcFileStream);
        SpecLexer lexer(&inStream);
        antlr4::CommonTokenStream tokens(&lexer);

        // tokens.fill();
        // for (auto token : tokens.getTokens()) {
        //     std::cout << token->toString() << std::endl;
        // }

        SpecParser parser(&tokens);
        antlr4::tree::ParseTree *tree = parser.file();

        visit(tree);

        std::cout.flush();
    }

    void SourceSpecTransformer::test(char *inputPath)
    {

        std::ifstream srcFileStream;
        srcFileStream.open(inputPath);

        antlr4::ANTLRInputStream inStream(srcFileStream);
        SpecLexer lexer(&inStream);
        antlr4::CommonTokenStream tokens(&lexer);

        /*
        std::cout << "TOKENS: " << std::endl;
        tokens.fill();
        for (auto token : tokens.getTokens()) {
            std::cout << token->toString() << std::endl;
            // std::cout << token->getChannel() << " - " << token->getText() << " - " << token->getType() << std::endl;
        }
        */

        SpecParser parser(&tokens);
        antlr4::tree::ParseTree *tree = parser.file();

        std::cout << tree->toStringTree(&parser) << std::endl
                  << std::endl;

        std::cout.flush();
    }

    std::any SourceSpecTransformer::visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx)
    {
        std::cout << "Entering SpecContractBlock: " << std::endl;
        auto res = visitChildren(ctx);
        std::cout << "Exiting SpecContractBlock: " << std::endl;
        return res;
    }

    std::any SourceSpecTransformer::visitRequiresClause(SpecParser::RequiresClauseContext *ctx)
    {
        auto expr = ctx->specExpression();
        std::cout << "Requires clause: " << getExprAsStr(expr) << std::endl;
        return 0;
        // return visitChildren(ctx);
    }

    std::any SourceSpecTransformer::visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx)
    {
        auto expr = ctx->specExpression();
        std::cout << "Requires clause: " << getExprAsStr(expr) << std::endl;
        return 0;
    }

    std::string SourceSpecTransformer::getExprAsStr(SpecParser::SpecExpressionContext *exprCtx) {
        
        auto startToken = exprCtx->start;
        auto stopToken = exprCtx->stop;
        auto charStream = startToken->getTokenSource()->getInputStream();

        antlr4::misc::Interval i(startToken->getStartIndex(), stopToken->getStopIndex());
        return charStream->getText(i);
    }

}