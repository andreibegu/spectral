#ifndef PALLAS_IRLOOKUP_H
#define PALLAS_IRLOOKUP_H

#include <memory>
#include <set>

#include <llvm/ADT/SetVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>

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
         */
        std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>>
        getSrcVariablesForFunction(const llvm::Function &fun);

        /**
         * Collects the DILocalVariable-nodes that represent variables that can
         * be referenced in the loop-invariant of the given loop.
         *
         * Global variables are currently ignored.
         */
        std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>>
        getVarsForLoopInv(llvm::Function *parentFunc,
                          const llvm::BasicBlock *loopHeader);

        /**
         * Collects the DILocalVariable-nodes that represent variables that can
         * be referenced in a specification-statement (e.g. assert) that is
         * attached to the given instruction.
         *
         * Global variables are currently ignored.
         */
        std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>> getVarsForStatement(
            llvm::Instruction *inst);

        /**
         * Returns the effective return-type for an llvm-function.
         * I.e. the fact that returning a struct-type is encoded as a function-
         * argument with the 'sret'-attribute is considered.
         */
        llvm::Type *getReturnType(const llvm::Function &f);
    }

}

#endif // PALLAS_IRLOOKUP