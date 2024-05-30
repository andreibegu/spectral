#ifndef PALLAS_IRLOOKUP_H
#define PALLAS_IRLOOKUP_H

#include <memory>
#include <set>

#include <llvm/ADT/SetVector.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>

namespace sourcespec2ir
{
    namespace utils 
    {
        /**
         * Helper functions that look-up or collect information from the LLVM-IR.
         *
         * Utilizes the debug-information of the LLVM-IR to retrieve the source-level variables.
         *
         * TODO: Since the dubug-intrinsics will be deprecated with LLVM-19 (https://llvm.org/docs/RemoveDIsDebugInfo.html) this will have to
         * be migrated once we switch to LLVM-19.
         */

        /**
         * Collects the DILocalVariable-nodes that are references from the given function 'fun'.
         * Parameter 'argsOnly' determines if only function arguments are considered.
         * The detection of function-arguments assumes that the mem2reg pass has been run on the IR of the representation.
         * Currently, global variables are not returned when argsOnly is set to true.
         */
        std::unique_ptr<std::set<llvm::DILocalVariable*>> getSrcVariablesForFunction(const llvm::Function &fun, const bool argsOnly);

    }

}

#endif // PALLAS_IRLOOKUP