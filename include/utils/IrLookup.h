#ifndef PALLAS_IRLOOKUP_H
#define PALLAS_IRLOOKUP_H

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>

namespace sourcespec2ir
{
    namespace utils
    {
        /**
         * Helper functions that look-up or collect information from the LLVM-IR.
         *
         * TODO: Since the dubug-intrinsics will be deprecated with LLVM-19
         * (https://llvm.org/docs/RemoveDIsDebugInfo.html) this will have to
         * be migrated once we switch to LLVM-19.
         */

        /**
         * Returns the effective return-type for an llvm-function.
         * I.e. the fact that returning a struct-type is encoded as a function-
         * argument with the 'sret'-attribute is considered.
         */
        llvm::Type *getReturnType(const llvm::Function &f);
    }

}

#endif // PALLAS_IRLOOKUP