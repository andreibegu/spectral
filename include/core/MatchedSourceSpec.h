#ifndef PALLAS_MATCHEDSOURCESPEC_H
#define PALLAS_MATCHEDSOURCESPEC_H

#include <GhostMethod.h>
#include "wrapper/SrcLocation.h"
#include "wrapper/WrapperSignature.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>

#include <optional>

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
        Assume,
        Fold,
        Unfold,
        GhostAssign
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
        bool isExternal;

        MatchedFContractStatement(ContractStatementType type, std::string expr,
                                  const SrcRange &srcLoc,
                                  const SrcLocation &exprLoc,
                                  bool isExternal)
            : type(type), expr(expr), srcLoc(srcLoc), exprLoc(exprLoc),
              isExternal(isExternal)
        {
        }
    };

    /**
     * Struct that represents a definition of a ghost argument.
     */
    struct GhostArgumentDef
    {
        const std::string defStr;
        const std::string argName;
        const SrcRange srcLoc;
        const SrcLocation defStartLoc;

        GhostArgumentDef(const std::string &defStr, const std::string &argName,
                         const SrcRange &srcLoc, const SrcLocation &defStartLoc)
            : defStr(defStr), argName(argName),
              srcLoc(srcLoc), defStartLoc(defStartLoc)
        {
        }
    };

    enum ContractType
    {
        Normal,
        External,
        Generated
    };

    /**
     * A struct that represents a function contract-block
     *  that has been matched to an IR-function.
     * The order of statement's in the source must be maintained in the vector!
     */
    struct MatchedFunctionContract
    {
        llvm::Function *function;
        llvm::SmallVector<GhostArgumentDef, 4> givenArgs;
        llvm::SmallVector<GhostArgumentDef, 4> yieldsArgs;
        llvm::SmallVector<std::unique_ptr<MatchedFContractStatement>> statements;
        SrcRange srcLoc;
        const ContractType type;
        bool isPure = false;
        bool isAssumed = false;
        std::optional<WrapperSignature> wrapperSignature = std::nullopt;

        MatchedFunctionContract(
            llvm::Function *function,
            const SrcRange &srcLoc, const ContractType &type)
            : function(function), srcLoc(srcLoc), type(type)
        {
        }

        /**
         * Returns a pointer to the given argument with the given name.
         * If no argument with the provided name exists, a nullptr is returned.
         */
        const GhostArgumentDef *getGivenArg(const std::string &name) const;

        /**
         * Returns a pointer to the yields argument with the given name.
         * If no argument with the provided name exists, a nullptr is returned.
         */
        const GhostArgumentDef *getYieldsArg(const std::string &name) const;

        /**
         * Returns a pointer to a ghost argument with the given name.
         * If no argument with the provided name exists, a nullptr is returned.
         */
        const GhostArgumentDef *getGhostArg(const std::string &name) const;

        void addStatement(std::unique_ptr<MatchedFContractStatement> stmnt)
        {
            statements.push_back(std::move(stmnt));
        }
    };

    /**
     * A struct that represents a function contract that belongs to a
     * ghost method or function.
     * The order of statement's in the source must be maintained in the vector!
     */
    struct MatchedGhostContract
    {
        GhostMethod *method;
        llvm::SmallVector<std::unique_ptr<MatchedFContractStatement>> statements;
        SrcRange srcLoc;
        const WrapperSignature wrapperSig;

        MatchedGhostContract(GhostMethod *method, const SrcRange &srcLoc,
                             const WrapperSignature &wrapperSig)
            : method(method), srcLoc(srcLoc), wrapperSig(wrapperSig)
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
        // If this statement is a ghost assignment, this holds the name of
        // the ghost-variable
        std::optional<std::string> ghostVarName;

        MatchedSpecStatement(SpecStatementType type, std::string expr,
                             const SrcRange &srcLoc, const SrcLocation &exprLoc,
                             std::optional<std::string> ghostVarName = std::nullopt)
            : type(type), expr(expr), srcLoc(srcLoc), exprLoc(exprLoc),
              ghostVarName(ghostVarName)
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
     * A struct that represents a binding of an expression to a given-variable
     * or of a yields variable to another ghost variable.
     */
    struct MatchedGhostVarBinding
    {
        // The name of the ghost argument
        std::string ghostArg;
        // String repr. of the expression
        std::string exprStr;
        // The start- and end-position of the binding in the source-file
        SrcRange srcLoc;
        // The start-position of the expression in the source-file.
        SrcLocation exprLoc;

        MatchedGhostVarBinding(const std::string &ghostArg,
                               const std::string &exprStr,
                               const SrcRange &srcLoc,
                               const SrcLocation &exprLoc)
            : ghostArg(ghostArg),
              exprStr(exprStr),
              srcLoc(srcLoc),
              exprLoc(exprLoc)
        {
        }
    };

    /**
     * A struct that represents a set of bindings to ghost-arguments that are
     * matched to a function-call.
     */
    struct MatchedGhostArgBindingBlock
    {
        llvm::CallInst *call;
        llvm::SmallVector<MatchedGhostVarBinding, 2> bindings;
        SrcRange srcLoc;

        MatchedGhostArgBindingBlock(
            llvm::CallInst *call, const SrcRange &srcLoc)
            : call(call), srcLoc(srcLoc)
        {
        }

        void addBinding(MatchedGhostVarBinding binding)
        {
            bindings.push_back(std::move(binding));
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
        llvm::SmallVector<std::unique_ptr<MatchedSpecStatementBlock>>
            matchedStmntBlocks;
        llvm::SmallVector<std::unique_ptr<MatchedGhostContract>>
            matchedGhostContracts;
        llvm::SmallVector<std::unique_ptr<MatchedGhostArgBindingBlock>>
            matchedGivenBindings;
        llvm::SmallVector<std::unique_ptr<MatchedGhostArgBindingBlock>>
            matchedYieldsBindings;

        llvm::SmallVector<std::unique_ptr<GhostMethod>> ghostMethods;

        MatchedSpecifications()
            : matchedFContracts(),
              matchedLoopInvariants(),
              matchedStmntBlocks(),
              matchedGhostContracts(),
              matchedGivenBindings(),
              matchedYieldsBindings(),
              ghostMethods()
        {
        }

        /**
         * Adds the specifications from 'specs2' to this object.
         */
        void merge(std::unique_ptr<MatchedSpecifications> specs2);

        /**
         * Checks that no two specification-elements are matched to the same
         * LLVM construct.
         */
        bool hasDuplicateMappings() const;

        /**
         * Returns vector of pointers to all ghost methods.
         */
        llvm::SmallVector<GhostMethod *> getGhostMethods() const;

        /**
         * If a contract is matched to the given function, return this contract.
         * Otherwise returns a nullptr.
         */
        const MatchedFunctionContract *getContractForFunc(
            const llvm::Function &f) const;
    };

}

#endif // PALLAS_MATCHEDSOURCESPEC_H