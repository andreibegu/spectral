#include "SpecDeclarationCollector.h"
#include "ParsingUtils.h"

namespace sourcespec2ir
{
    SpecDeclarationCollector::SpecDeclarationCollector(SpecParser::FileContext *specParseTree)
        : specParseTree(specParseTree)
    {
    }

    std::unique_ptr<llvm::SmallVector<SpecDeclaration>>
    SpecDeclarationCollector::collectDeclarations()
    {
        declarations.reset();
        declarations = std::make_unique<llvm::SmallVector<SpecDeclaration>>();
        visit(specParseTree);
        return std::move(declarations);
    }

    std::any SpecDeclarationCollector::visitDeclarationClause(
        SpecParser::DeclarationClauseContext *ctx)
    {
        auto *specExpr = ctx->specExpression();
        auto declStr = utils::getCtxAsStr(*specExpr);
        declarations->push_back(
            SpecDeclaration(declStr, specExpr->start->getLine()));
        return 0;
    }

}