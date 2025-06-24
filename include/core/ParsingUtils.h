#ifndef PALLAS_PARSINGUTILS_H
#define PALLAS_PARSINGUTILS_H

#include "SpecParser.h"
#include "wrapper/SrcLocation.h"

#include <string>

namespace sourcespec2ir::utils
{
    /**
     * Returns the textual representation of the given parser-rule 
     * including whitespace.
     */
    std::string getCtxAsStr(const antlr4::ParserRuleContext &argCtx);

    /**
     * Extracts the source-location from a given parser-rule context and
     * returns it as a SrcRange.
     */
    SrcRange getLocFromCtx(const antlr4::ParserRuleContext *ctx,
                           const std::string &file);

    /**
     * Returns the location in the source-file where the given expression
     * starts.
     */
    SrcLocation getExprStartLoc(
        const SpecParser::SpecExpressionContext &exprCtx,
        const std::string &file);
}

#endif // PALLAS_PARSINGUTILS_H