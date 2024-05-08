#ifndef PALLAS_SOURCESPECTRANSFORMER_H
#define PALLAS_SOURCESPECTRANSFORMER_H

#include <string>
#include "SpecParserBaseVisitor.h"

namespace sourcespec2ir
{

    class SourceSpecTransformer : SpecParserBaseVisitor
    {
    public:
        void printSpecifications(char *inputPath);
        void test(char *inputPath);

        // Overwritten visitor-methods
        std::any visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx) override;
        std::any visitRequiresClause(SpecParser::RequiresClauseContext *ctx) override;
        std::any visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx) override; 

    private:
        /**
         * Retrieving the textual representation of the specification expressions using getText() removes the whitespaces. 
         * This method returns the full textual representatoin including whitespaces by using the underlying token-stream. 
         */
        std::string getExprAsStr(SpecParser::SpecExpressionContext *exprCtx);
    };

}

#endif // PALLAS_SOURCESPECTRANSFORMER_H