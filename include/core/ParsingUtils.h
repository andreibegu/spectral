#ifndef PALLAS_PARSINGUTILS_H
#define PALLAS_PARSINGUTILS_H

#include "SpecParser.h"
#include "wrapper/SrcLocation.h"

#include <string>

namespace sourcespec2ir::utils
{
    // TODO: Merge this with the SpecParserWrapper and get rid of the file-args.

    /**
     * Retrieving the textual representation of the specification
     * expressions using getText() removes the whitespaces.
     * This method returns the full textual representatoin including
     * whitespaces by using the underlying token-stream.
     */
    std::string getExprAsStr(const SpecParser::SpecExpressionContext &exprCtx);

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