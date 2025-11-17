#include "SpecPredicateCollector.h"

#include <cassert>

#include "ParsingUtils.h"

namespace sourcespec2ir
{
    SpecPredicateCollector::SpecPredicateCollector(
        SpecParser::FileContext &specParseTree, const std::string &file,
        const std::string &md5Checksum)
        : specParseTree(specParseTree), file(file), md5Checksum(md5Checksum)
    {
    }

    std::unique_ptr<llvm::SmallVector<SpecPredicate>>
    SpecPredicateCollector::collect()
    {
        assert(predicates == nullptr);
        predicates =
            std::make_unique<llvm::SmallVector<SpecPredicate>>();
        visit(&specParseTree);
        return std::move(predicates);
    }

    std::any SpecPredicateCollector::visitPredicateDef(
        SpecParser::PredicateDefContext *ctx)
    {
        assert(predicates != nullptr);

        auto loc = utils::getLocFromCtx(ctx, file, md5Checksum);
        auto sig = utils::getCtxAsStr(*ctx->predicateSig());
        auto body = utils::getCtxAsStr(*ctx->specExpression());

        predicates->push_back(SpecPredicate(sig, body, loc));
        return 0;
    }
}