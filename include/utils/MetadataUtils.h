#ifndef PALLAS_MDUTILS_H
#define PALLAS_MDUTILS_H

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

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

        /** 
         * Adds metadata marker to mark the passed function as a ghost wrapper. 
         */
        void markGhostWrapper(llvm::Function &f, llvm::LLVMContext &context);

        /**
         * Checks if the given function is marked as a ghost wrapper.
         */
        bool isGhostWrapper(const llvm::Function &f);

        /**
         * Adds a metadata-node to the function that marks it as a
         * definition of a predicate.
         */
        void markPredicateDef(llvm::Function &f, llvm::LLVMContext &context, 
            bool isInline);

        /**
         * Adds a metadata-node to the function that marks it as a
         * ghost method.
         */
        void markGhostMethod(llvm::Function &f, llvm::LLVMContext &context);

        /**
         * Checks if the given function is marked as a ghost method.
         */
        bool isGhostMethod(llvm::Function &f);

        /**
         * Checks if the given function is marked as a predicate definition.
         */
        bool isPredicateDef(const llvm::Function &f);

        /**
         * Checks if scope is a sub-scope of the parentScope.
         */
        bool isSubscope(const llvm::DIScope *parentScope,
                        const llvm::DIScope *scope);

        /**
         * Returns the DIScope of the first instruction in the basic block that
         * has a debug-scope
         */
        llvm::DILocalScope *getScopeOfBasicBlock(const llvm::BasicBlock &bb);

        /**
         * Returns the DIScope of the given instruction (if it has one)
         */
        llvm::DILocalScope *getScopeOfInstruction(const llvm::Instruction &i);

    }
}

#endif // PALLAS_MDUTILS_H