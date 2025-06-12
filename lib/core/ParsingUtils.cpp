#include "ParsingUtils.h"

namespace sourcespec2ir::utils
{
    std::string getExprAsStr(const SpecParser::SpecExpressionContext &eCtx)
    {
        auto startToken = eCtx.start;
        auto stopToken = eCtx.stop;
        auto charStream = startToken->getTokenSource()->getInputStream();

        antlr4::misc::Interval i(startToken->getStartIndex(),
                                 stopToken->getStopIndex());
        return charStream->getText(i);
    }

    SrcRange getLocFromCtx(const antlr4::ParserRuleContext *ctx,
                           const std::string &file)
    {
        // We change the column-numbering to start with 1 to be consistent with
        // the line-info of LLVM
        return SrcRange(ctx->start->getLine(), ctx->start->getCharPositionInLine() + 1,
                        ctx->stop->getLine(), ctx->stop->getCharPositionInLine() + 1,
                        file);
    }

    SrcLocation getExprStartLoc(
        const SpecParser::SpecExpressionContext &exprCtx,
        const std::string &file)
    {
        return SrcLocation(exprCtx.start->getLine(),
                           exprCtx.start->getCharPositionInLine(), file);
    }
}