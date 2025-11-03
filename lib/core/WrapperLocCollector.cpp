#include "WrapperLocCollector.h"

#include <cassert>

#include "ParsingUtils.h"

namespace sourcespec2ir
{
    WrapperLocCollector::WrapperLocCollector(
        SpecParser::FileContext &specParseTree, const std::string &file,
        const std::string &md5Checksum)
        : specParseTree(specParseTree), file(file), md5Checksum(md5Checksum)
    {
    }

    std::unique_ptr<llvm::SmallVector<WrapperLocMarker>>
    WrapperLocCollector::collectLocationMarkers()
    {
        assert(wrapperLocations == nullptr);
        wrapperLocations =
            std::make_unique<llvm::SmallVector<WrapperLocMarker>>();
        visit(&specParseTree);
        return std::move(wrapperLocations);
    }

    std::any WrapperLocCollector::visitWrapperLocationMarker(
        SpecParser::WrapperLocationMarkerContext *ctx)
    {
        assert(wrapperLocations != nullptr);

        // Get location of the annotation
        auto locRange = utils::getLocFromCtx(ctx, file, md5Checksum);
        // Get text of the matching string
        auto *sExpr = ctx->specExpression();
        std::optional<std::string> matchingStr = std::nullopt;
        if (ctx->specExpression() != nullptr)
            matchingStr = std::make_optional(utils::getCtxAsStr(*sExpr));

        wrapperLocations->push_back(
            WrapperLocMarker(matchingStr, locRange));
        return 0;
    }

}