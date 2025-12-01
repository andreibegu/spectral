#include "SpecPredicateCollector.h"

#include <cassert>
#include <llvm/Support/raw_ostream.h>
#include "ParsingUtils.h"

namespace sourcespec2ir
{
    SpecPredicateCollector::SpecPredicateCollector(
        SpecParser::FileContext &specParseTree, const std::string &file,
        const std::string &md5Checksum, const bool printInfo)
        : specParseTree(specParseTree),
          file(file),
          md5Checksum(md5Checksum),
          printInfo(printInfo)
    {
    }

    std::unique_ptr<llvm::SmallVector<SpecPredicate, 16>>
    SpecPredicateCollector::collect()
    {
        assert(predicates == nullptr);
        predicates =
            std::make_unique<llvm::SmallVector<SpecPredicate, 16>>();
        visit(&specParseTree);
        return std::move(predicates);
    }

    std::any SpecPredicateCollector::visitPredicateDef(
        SpecParser::PredicateDefContext *ctx)
    {
        assert(predicates != nullptr);

        auto srcLoc = utils::getLocFromCtx(ctx, file, md5Checksum);
        auto exprLoc = utils::getExprStartLoc(*ctx->specExpression(), file,
                                              md5Checksum);
        auto isInline = ctx->INLINE() != nullptr;
        auto name = utils::getCtxAsStr(*ctx->predName());
        auto args = utils::getCtxAsStr(*ctx->predicateSig());
        auto body = utils::getCtxAsStr(*ctx->specExpression());

        predicates->push_back(
            SpecPredicate(isInline, name, args, body, srcLoc, exprLoc));

        if (printInfo)
        {
            llvm::outs() << "Found predicate at "
                         << srcLoc.getStartLine() << "::" << srcLoc.getStartCol()
                         << " - "
                         << srcLoc.getEndLine() << "::" << srcLoc.getEndCol()
                         << "\n";
        }

        return 0;
    }
}