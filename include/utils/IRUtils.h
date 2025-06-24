#ifndef PALLAS_IRUTILS_H
#define PALLAS_IRUTILS_H

#include <llvm/ADT/SmallSet.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>

namespace sourcespec2ir
{
    namespace utils
    {
        /**
         * Removes the given arguments from the given wrapper function.
         * Assumes, that the wrapper-function is never called or invoked.
         * Internally, this makes a copy of the old function and deletes the old
         * function afterwards. I.e. all pointers to wFunc become invalid.
         */
        void removeArgsFromWrapper(
            llvm::Function *wFunc,
            llvm::SmallSet<llvm::Argument *, 2> &argsToRemove);

        /**
         * If the functionshas a DISubprogram that is split into declaration 
         * and definition, return the subprogram of the declaration. 
         * Otherwise return subprogram that is directly attached to function.
         */
        llvm::DISubprogram *getDeclSubprog(const llvm::Function &f);
    }

}

#endif // PALLAS_IRUTILS_H