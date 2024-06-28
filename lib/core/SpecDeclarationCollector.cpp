#include "core/SpecDeclarationCollector.h"
#include "antlr4-runtime.h"

namespace sourcespec2ir
{
    SpecDeclarationCollector::SpecDeclarationCollector(SpecParser::FileContext *specParseTree)
        : specParseTree(specParseTree)
    {}

    std::unique_ptr<llvm::SmallVector<std::string>> SpecDeclarationCollector::collectDeclarations()
    {
        declarations.reset();
        declarations = std::make_unique<llvm::SmallVector<std::string>>();
        visit(specParseTree);
        return std::move(declarations);
    }

    std::any SpecDeclarationCollector::visitDeclarationClause(SpecParser::DeclarationClauseContext *ctx)
    {
        // Get the text of the declaration-expression and append it to the vector. 
        auto startToken = ctx->specExpression()->start;
        auto stopToken = ctx->specExpression()->stop;
        auto charStream = startToken->getTokenSource()->getInputStream();

        antlr4::misc::Interval i(startToken->getStartIndex(), stopToken->getStopIndex());
        auto declStr = charStream->getText(i);
        declarations->push_back(declStr);
        return 0;
    }
    
}