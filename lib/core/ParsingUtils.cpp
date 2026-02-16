#include "ParsingUtils.h"

namespace sourcespec2ir::utils
{
    std::string getCtxAsStr(const antlr4::ParserRuleContext &argCtx)
    {
        auto startToken = argCtx.start;
        auto stopToken = argCtx.stop;
        auto charStream = startToken->getTokenSource()->getInputStream();

        antlr4::misc::Interval i(startToken->getStartIndex(),
                                 stopToken->getStopIndex());
        return charStream->getText(i);
    }

    SrcRange getLocFromCtx(const antlr4::ParserRuleContext *ctx,
                           const std::string &file,
                           const std::string &md5Checksum)
    {
        // We change the column-numbering to start with 1 to be consistent with
        // the line-info of LLVM
        return SrcRange(ctx->start->getLine(), ctx->start->getCharPositionInLine() + 1,
                        ctx->stop->getLine(), ctx->stop->getCharPositionInLine() + 1,
                        file, md5Checksum);
    }

    SrcLocation getExprStartLoc(
        const SpecParser::SpecExpressionContext &exprCtx,
        const std::string &file, const std::string &md5Checksum)
    {
        return SrcLocation(exprCtx.start->getLine(),
                           exprCtx.start->getCharPositionInLine(),
                           file, md5Checksum);
    }

    SrcLocation getGhostImplStartLoc(
        const SpecParser::GhostImplContext &implCtx,
        const std::string &file, const std::string &md5Checksum)
    {
        return SrcLocation(implCtx.start->getLine(),
                           implCtx.start->getCharPositionInLine(),
                           file, md5Checksum);
    }
}