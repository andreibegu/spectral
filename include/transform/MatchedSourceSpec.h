#ifndef PALLAS_MATCHEDSOURCESPEC_H
#define PALLAS_MATCHEDSOURCESPEC_H

#include "SpecParser.h"

#include <llvm/IR/Function.h>

namespace sourcespec2ir
{

    /**
     * A struct that represents a function contract that has been matched to an IR-function.
     */
    struct MatchedFunctionContract
    {
        llvm::Function *function;
        SpecParser::SpecContractBlockContext *ctx;

        MatchedFunctionContract(llvm::Function *function, SpecParser::SpecContractBlockContext *ctx)
            : function(function),
              ctx(ctx)
        {}
    };

}

#endif // PALLAS_MATCHEDSOURCESPEC_H