#ifndef PALLAS_MATCHEDSOURCESPEC_H
#define PALLAS_MATCHEDSOURCESPEC_H

#include "wrapper/SrcLocation.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Metadata.h>

namespace sourcespec2ir
{

    /**
     * Enum to identify different types of contract statements.
     */
    enum ContractStatementType
    {
        Requires,
        Ensures
    };

    /**
     * Enum to represent the different types of specification-statements
     */
    enum SpecStatementType
    {
        Assert,
        Assume
    };

    /**
     * Struct that represents a contract statement from the source-file
     * that has been matched to the IR.
     */
    struct MatchedFContractStatement
    {
        ContractStatementType type;
        // The source-language expression of the statement
        std::string expr;
        // The location of the contract statement in the source-file
        SrcRange srcLoc;
        // The start-position of the statements expression in the source-file.
        SrcLocation exprLoc;

        MatchedFContractStatement(ContractStatementType type, std::string expr,
                                  const SrcRange &srcLoc,
                                  const SrcLocation &exprLoc)
            : type(type), expr(expr), srcLoc(srcLoc), exprLoc(exprLoc)
        {
        }
    };

    /**
     * A struct that represents a function contract-block
     *  that has been matched to an IR-function.
     * The order of statement's in the source must be maintained in the vector!
     */
    struct MatchedFunctionContract
    {
        llvm::Function *function;
        llvm::SmallVector<std::unique_ptr<MatchedFContractStatement>> statements;
        SrcRange srcLoc;
        bool isPure = false;
        bool isAssumed = false;

        MatchedFunctionContract(llvm::Function *function, const SrcRange &srcLoc)
            : function(function), srcLoc(srcLoc)
        {
        }

        void addStatement(std::unique_ptr<MatchedFContractStatement> stmnt)
        {
            statements.push_back(std::move(stmnt));
        }
    };

    /**
     * Struct that represents a loop invariant that has been matched to a loop
     * in the IR.
     */
    struct MatchedLoopInvariant
    {
        std::string expr;
        SrcRange srcLoc;
        // The start-position of the invariant's expression in the source-file.
        SrcLocation exprLoc;

        MatchedLoopInvariant(std::string expr, const SrcRange &srcLoc,
                             const SrcLocation &exprLoc)
            : expr(expr), srcLoc(srcLoc), exprLoc(exprLoc)
        {
        }
    };

    /**
     * Struct that represents a block of loop-invariants that have been matched
     * to a loop in the IR.
     * The order of invariants in the source must be maintained in the vector!
     */
    struct MatchedLoopInvariantBlock
    {
        llvm::Function *parentFunction;
        llvm::MDTuple *loopID;
        llvm::BasicBlock *loopHeader;
        llvm::SmallVector<std::unique_ptr<MatchedLoopInvariant>> invariants;
        SrcRange srcLoc;

        MatchedLoopInvariantBlock(llvm::Function *parentFunction,
                                  llvm::MDTuple *loopID,
                                  llvm::BasicBlock *loopHeader,
                                  const SrcRange &srcLoc)
            : parentFunction(parentFunction), loopID(loopID),
              loopHeader(loopHeader), invariants(), srcLoc(srcLoc)
        {
        }

        void addInvariant(std::unique_ptr<MatchedLoopInvariant> inv)
        {
            invariants.push_back(std::move(inv));
        }
    };

    /**
     * Struct that represents a specification statement from the source-file.
     */
    struct MatchedSpecStatement
    {
        SpecStatementType type;
        // The source-language expression of the statement
        std::string expr;
        // The start- and end-position of the statement in the source-file
        SrcRange srcLoc;
        // The start-position of the expression in the source-file.
        SrcLocation exprLoc;

        MatchedSpecStatement(SpecStatementType type, std::string expr,
                             const SrcRange &srcLoc, const SrcLocation &exprLoc)
            : type(type), expr(expr), srcLoc(srcLoc), exprLoc(exprLoc)
        {
        }
    };

    /**
     * A struct that represents a block of specification-statements that have
     * been matched to an instruction in the IR.
     * The order of statements in the source must be maintained in the vector!
     */
    struct MatchedSpecStatementBlock
    {
        llvm::Instruction *inst;
        llvm::SmallVector<std::unique_ptr<MatchedSpecStatement>> statements;
        SrcRange srcLoc;

        MatchedSpecStatementBlock(llvm::Instruction *inst, const SrcRange &srcLoc)
            : inst(inst), srcLoc(srcLoc)
        {
        }

        void addStatement(std::unique_ptr<MatchedSpecStatement> stmnt)
        {
            statements.push_back(std::move(stmnt));
        }
    };

    /**
     * Struct that bundles the matched specifications.
     * Contains matched function-contracts and loop-invariants.
     *
     * Takes ownership of the matched specifications.
     */
    struct MatchedSpecifications
    {
        llvm::SmallVector<std::unique_ptr<MatchedFunctionContract>>
            matchedFContracts;
        llvm::SmallVector<std::unique_ptr<MatchedFunctionContract>>
            matchedExternalContracts;
        llvm::SmallVector<std::unique_ptr<MatchedLoopInvariantBlock>>
            matchedLoopInvariants;
        llvm::SmallVector<std::unique_ptr<MatchedSpecStatementBlock>>
            matchedStmntBlocks;

        MatchedSpecifications()
            : matchedFContracts(),
              matchedExternalContracts(),
              matchedLoopInvariants(),
              matchedStmntBlocks()
        {
        }

        /**
         * Adds the specifications from 'specs2' to this object.
         */
        void merge(std::unique_ptr<MatchedSpecifications> specs2);
    };

}

#endif // PALLAS_MATCHEDSOURCESPEC_H