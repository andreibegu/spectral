#ifndef PALLAS_SPECENCODER_H
#define PALLAS_SPECENCODER_H

#include "IrSpecConstants.h"
#include "MatchedSourceSpec.h"
#include "wrapper/MatchedWrapperFunctions.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>

#include <map>

namespace sourcespec2ir
{

    /**
     * Encoded specifications into metadata-nodes in the given module.
     *
     * Does not take ownership of the passed context.
     */
    class SpecEncoder
    {
    public:
        SpecEncoder(llvm::LLVMContext &irContext, llvm::Module &irModule,
                    const MatchedWrapperFunctions &matchedWrappers);

        virtual ~SpecEncoder() = default;

        /**
         * Invokes the specification-unification on the given module.
         */
        void encodeSpecifications(const MatchedSpecifications &specs);

    private:
        llvm::LLVMContext &irContext;
        llvm::Module &irModule;
        const MatchedWrapperFunctions &matchedWrappers;

        std::map<const GhostArgumentDef *, llvm::Metadata *> ghostArgDefMap;

        /**
         * Encodes a block of function-contract statements into metadata
         */
        void encodeFunctionContractBlock(
            const MatchedFunctionContract &contract);

        /**
         * Encodes a statement of a function contract into a metadata-node.
         * Returns a pointer to the new MD-node.
         */
        llvm::MDNode *encodeFunctionContractStatement(
            const MatchedFContractStatement &stmnt,
            const MatchedFunctionContract *parentContract,
            const bool inGhostContract);

        /**
         * Encodes a block of loop-invariants into metadata.
         */
        void encodeLoopInvBlock(
            const MatchedLoopInvariantBlock &invBlock,
            const MatchedFunctionContract *parentContract);

        /**
         * Encodes a single loop-invariant into a metadata-node.
         * The new Metadata node is added to the back of the given vector.
         */
        llvm::MDNode *encodeLoopInv(
            const MatchedLoopInvariant &inv,
            const MatchedFunctionContract *parentContract);

        /**
         * Encodes a block of specification-statements into metadata.
         */
        void encodeStmntBlock(
            const MatchedSpecStatementBlock &stmntBlock,
            const MatchedFunctionContract *parentContract);

        /**
         * Encodes a statement from a block of spec-statements into a md-node.
         * Returns a pointer to the new MD-node.
         */
        llvm::MDNode *encodeSpecStatement(
            const MatchedSpecStatement &stmnt,
            const MatchedFunctionContract *parentContract);

        /**
         * Encodes a ghost contract into metadata
         */
        void encodeGhostContract(const MatchedGhostContract &contract);

        /**
         * Encodes a block of bindings to given-arguments.
         */
        void encodeGivenBindingBlock(
            const MatchedGhostArgBindingBlock &bBlock,
            const MatchedFunctionContract &calleeContract,
            const MatchedFunctionContract *parentContract);

        /**
         * Encodes an individual binding from a block of given-arg bindings into
         * a md-node.
         * Returns a pointer to the new MD-node.
         */
        llvm::MDNode *encodeGivenBinding(
            const MatchedGhostVarBinding &binding,
            const MatchedFunctionContract &calleeContract,
            const MatchedFunctionContract *parentContract);

        /**
         * Encodes a block of bindings to yields-arguments.
         */
        void encodeYieldsBindingBlock(
            const MatchedGhostArgBindingBlock &bBlock,
            const MatchedFunctionContract &parentContract,
            const MatchedFunctionContract &calleeContract);

        /**
         * Encodes an individual binding from a block of yields-arg bindings
         * into a md-node.
         * Returns a pointer to the new MD-node.
         */
        llvm::MDNode *encodeYieldsBinding(
            const MatchedGhostVarBinding &binding,
            const MatchedFunctionContract &parentContract,
            const MatchedFunctionContract &calleeContract);

        /**
         * Encodes the definition of a ghost argument into a metadata-node
         */
        llvm::MDTuple *encodeGhostArgDef(const GhostArgumentDef &argDef);

        /**
         * Encode a set of ghost variables into a MDNode.
         */
        llvm::MDNode *encodeGhostVarSet(
            llvm::ArrayRef<std::string> gVarNames,
            const llvm::Function &irWrapper,
            const MatchedFunctionContract *parentContract);

        /**
         * Generates a MDTuple that encodes a source-location.
         */
        llvm::MDTuple *encodeSrcLoc(const SrcRange &srcLoc);

        /**
         * Maps the DILocalVariables in the given function that refer to an
         * argument to their name.
         */
        llvm::StringMap<llvm::DILocalVariable *> mapDIArgsToNames(
            const llvm::Function &f);

        void throwNoContractForFunc(const llvm::Function &func);
    };

}

#endif // PALLAS_SPECENCODER_H