#ifndef PALLAS_SOURCESPECMATCHER_H
#define PALLAS_SOURCESPECMATCHER_H

#include "MatchedSourceSpec.h"
#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"

#include <memory>
#include <utility>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Module.h>

namespace sourcespec2ir
{

    /**
     * Class that matches the contracts from the parse-tree of the source-file
     * to the corresponding constructs (functions, loops, instructions) in the
     * IR module.
     *
     * Takes ownership of the matched specifications. These are accessible via
     * the corresponding getters.
     * Does not take ownership of the parse-tree or the IR-module.
     */
    class SourceSpecMatcher : SpecParserBaseVisitor
    {
    public:
        SourceSpecMatcher(SpecParser::FileContext *specParseTree, llvm::Module *irModule);

        // Starts the matching-process 
        void matchSpecs();

        // Returns the number of matched function-contract-blocks
        size_t getNumMatchedFuncContracts();

        // Return matched function contract by index
        MatchedFunctionContract *getMatchedFuncContract(size_t idx);

        // Returns the number of matched loop-invariant-blocks
        size_t getNumMatchedLoopInv();

        // Return matched invariant-block by index
        MatchedLoopInvariantBlock *getMatchedLoopInv(size_t idx);

    protected:
        SpecParser::FileContext *specParseTree;
        llvm::Module *irModule;

        /**
         * Given the location of a specContractBlock in the source-file,
         * this method returns the name of the funcion that the specification
         * refers to.
         * TODO: Check if it makes more sense to return a pointer to the
         *       function instead of the name.
         */
        virtual std::string getContractBlockContext(
            size_t startLine, size_t startCol, size_t endLine, size_t endCol) = 0;

        /**
         * Given the location of a specification-block of loop-invariants,
         * returns a pointer to the loop that this specification refers to,
         * and a pointer to the function that the matched loop is contained in.
         * If the spec-block cannot be matched, a runtime exception it thrown.
         *
         * TODO: The loop-pointer might go out of scope. Return something
         *       else! E.g. a pointer to the llvm.loop md-node.
         */
        virtual std::pair<llvm::Function *, llvm::Loop *> matchLoopInvBlock(
            size_t startLine, size_t startCol, size_t endLine, size_t endCol) = 0;

    private:
        // Collections in which the matched specifications are collected
        llvm::SmallVector<std::unique_ptr<MatchedFunctionContract>> matchedFuncContracts;

        llvm::SmallVector<std::unique_ptr<MatchedLoopInvariantBlock>> matchedLoopInvariants;

        // Temporarily stores the pointer to the current function-contract
        // while the contained contract-statements are collected
        std::unique_ptr<sourcespec2ir::MatchedFunctionContract> contract = nullptr;

        // Overwritten visitor-methods
        std::any visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx) override;
        std::any visitSpecLoopInvariantBlock(SpecParser::SpecLoopInvariantBlockContext *ctx) override;
        std::any visitRequiresClause(SpecParser::RequiresClauseContext *ctx) override;
        std::any visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx) override;


        // Adds the given statement to the current function-contract 
        // (SourceSpecMatcher::contract).
        void addStatementToFContract(ContractStatementType type, 
                                     SpecParser::SpecExpressionContext *expr);

        /**
         * Retrieving the textual representation of the specification
         * expressions using getText() removes the whitespaces.
         * This method returns the full textual representatoin including
         * whitespaces by using the underlying token-stream.
         */
        std::string getExprAsStr(const SpecParser::SpecExpressionContext &exprCtx);
    };

}

#endif // PALLAS_SOURCESPECMATCHER_H