#ifndef PALLAS_MDUTILS_H
#define PALLAS_MDUTILS_H

#include <string>
#include <memory>

#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>


namespace sourcespec2ir
{
    namespace utils
    {
        /**
         * Adds a metadata-node to the function that marks it as a generated specification-function.
         */
        void markGeneratedSpecFunction(llvm::Function &f, llvm::LLVMContext &context);

        /**
         * Checks if the given function is marked as a generated specification-function.
         */
        bool isGeneratedSpecFunction(const llvm::Function &f);

    }
}

#endif // PALLAS_MDUTILS_H