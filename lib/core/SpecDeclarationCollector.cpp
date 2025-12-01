#include "SpecDeclarationCollector.h"
#include "ParsingUtils.h"

namespace sourcespec2ir
{
    SpecDeclarationCollector::SpecDeclarationCollector(
        SpecParser::FileContext *specParseTree, const std::string &file,
        const std::string &md5Checksum)
        : specParseTree(specParseTree),
          file(file),
          md5Checksum(md5Checksum)
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
        auto loc = utils::getLocFromCtx(ctx, file, md5Checksum);
        declarations->push_back(SpecDeclaration(declStr, loc));
        return 0;
    }

}