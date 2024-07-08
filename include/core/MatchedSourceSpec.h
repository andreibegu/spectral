#ifndef PALLAS_MATCHEDSOURCESPEC_H
#define PALLAS_MATCHEDSOURCESPEC_H

#include "SpecParser.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Metadata.h>

namespace sourcespec2ir
{

    /**
     * Enum to identify differtent types of contract statements.
     */
    enum ContractStatementType
    {
        Requires, // Requires-clause
        Ensures   // Ensures-clause
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

        MatchedFContractStatement(ContractStatementType type, std::string expr)
            : type(type), expr(expr)
        {
        }
    };

    /**
     * A struct that represents a function contract-block
     *  that has been matched to an IR-function.
     */
    struct MatchedFunctionContract
    {
        llvm::Function *function;
        llvm::SmallVector<std::unique_ptr<MatchedFContractStatement>> statements;

        MatchedFunctionContract(llvm::Function *function)
            : function(function)
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
     * TODO: Remove this and replace it with a vector of strings.
     */
    struct MatchedLoopInvariant
    {
        std::string expr;

        MatchedLoopInvariant(std::string expr)
            : expr(expr)
        {
        }
    };

    /**
     * Struct that represents a block of loop-invariants that have been matched
     * to a loop in the IR.
     */
    struct MatchedLoopInvariantBlock
    {
        llvm::Function *parentFunction;
        llvm::MDTuple *loopID;
        llvm::BasicBlock *loopHeader;

        llvm::SmallVector<std::unique_ptr<MatchedLoopInvariant>> invariants;

        MatchedLoopInvariantBlock(llvm::Function *parentFunction,
                                  llvm::MDTuple *loopID, llvm::BasicBlock *loopHeader)
            : parentFunction(parentFunction), loopID(loopID),
              loopHeader(loopHeader), invariants()
        {
        }

        void addInvariant(std::unique_ptr<MatchedLoopInvariant> inv)
        {
            invariants.push_back(std::move(inv));
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
        llvm::SmallVector<std::unique_ptr<MatchedLoopInvariantBlock>>
            matchedLoopInvariants;

        MatchedSpecifications()
            : matchedFContracts(),
              matchedLoopInvariants()
        {
        }
    };

}

#endif // PALLAS_MATCHEDSOURCESPEC_H