#ifndef PALLAS_MATCHEDWRAPPERFUNCTIONS_H
#define PALLAS_MATCHEDWRAPPERFUNCTIONS_H

#include <map>
#include <memory>

#include <llvm/ADT/SmallVector.h>

#include "wrapper/SrcWrapperFunction.h"
#include "MatchedSourceSpec.h"

namespace sourcespec2ir
{

    /**
     * A struct that bundles the generated wrapper functions and associates
     * them to the specification-constuct to which they belong to.
     * The struct takes ownership of the SrcWrapperFunctions,
     * but not of the matched specification-constructs.
     */
    struct MatchedWrapperFunctions
    {
        std::map<const MatchedFContractStatement *, std::unique_ptr<SrcWrapperFunction>>
            fContractWrappers;
        std::map<const MatchedLoopInvariant *, std::unique_ptr<SrcWrapperFunction>>
            loopInvWrappers;
        std::map<const MatchedSpecStatement *, std::unique_ptr<SrcWrapperFunction>>
            statementWrappers;
        std::map<const MatchedFContractStatement *, std::unique_ptr<SrcWrapperFunction>>
            ghostContractWrappers;
        std::map<const MatchedGhostVarBinding *, std::unique_ptr<SrcWrapperFunction>>
            givenBindingWrappers;
            
        MatchedWrapperFunctions();

        /**
         * Returns a vector that contains pointers to all wrapper-functions.
         */
        std::unique_ptr<llvm::SmallVector<SrcWrapperFunction *>> getWrappers();

        /**
         * Returns a vector with pointers to all wrapper-functions that belong
         * to an external contract.
         */
        llvm::SmallVector<SrcWrapperFunction *> getExternalWrappers();
    };
}

#endif // PALLAS_MATCHEDWRAPPERFUNCTIONS_H