#ifndef PALLAS_IRLOOKUP_H
#define PALLAS_IRLOOKUP_H

#include <memory>
#include <set>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>

namespace sourcespec2ir
{
    namespace utils 
    {
        /**
         * Helper functions that look-up or collect information from the LLVM-IR.
         *
         * Utilizes the debug-information of the LLVM-IR to retrieve 
         * the source-level variables.
         *
         * TODO: Since the dubug-intrinsics will be deprecated with LLVM-19 
         * (https://llvm.org/docs/RemoveDIsDebugInfo.html) this will have to
         * be migrated once we switch to LLVM-19.
         */

        /**
         * Collects the DILocalVariable-nodes that are references to variables
         * that are accessible from the contract of the given function. 
         * Global variables are currently not considered. 
         * The detection of function-arguments assumes that the mem2reg pass has 
         * been run on the IR of the representation.
         * // TODO: It should now work without mem2reg. Remove this note?
         */
        std::unique_ptr<std::set<llvm::DILocalVariable*>> getSrcVariablesForFunction(
            const llvm::Function &fun);

        /**
         * Collects the DILocalVariable-nodes that represent variables that can 
         * be referenced in the loop-invariant of the given loop. 
         * 
         * The current implementation assumes that the mem2reg-pass has been run
         * on the module. 
         * 
         * Global variables are currently ignored.
         */
        std::unique_ptr<std::set<llvm::DILocalVariable*>> getVarsForLoopInv(
           llvm::Function *parentFunc, const llvm::BasicBlock *loopHeader);
    }

}

#endif // PALLAS_IRLOOKUP