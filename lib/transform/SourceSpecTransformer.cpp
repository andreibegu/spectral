#include <iostream>
#include <cassert>

#include "transform/SourceSpecTransformer.h"
#include "utils/IRVariableCollector.h"
#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"

namespace sourcespec2ir
{

    void SourceSpecTransformer::printSpecifications(char *srcInputPath, char *irInputStream)
    {
        std::ifstream srcFileStream;
        srcFileStream.open(srcInputPath);

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

        // Test the IRVariableCollector
        IRVariableCollector varCollector("", irInputStream);

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
    }

    std::any SourceSpecTransformer::visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx)
    {
        auto expr = ctx->specExpression();
        std::cout << "Requires clause: " << getExprAsStr(expr) << std::endl;
        return 0;
    }

    std::any SourceSpecTransformer::visitInvariantClause(SpecParser::InvariantClauseContext *ctx)
    {
        std::cerr << "Loop invariants are currently not supported! " << std::endl;
        assert(false);
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