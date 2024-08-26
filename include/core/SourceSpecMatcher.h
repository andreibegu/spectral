#ifndef PALLAS_SOURCESPECMATCHER_H
#define PALLAS_SOURCESPECMATCHER_H

#include "MatchedSourceSpec.h"
#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"
#include "wrapper/SrcLocation.h"

#include <memory>
#include <utility>

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
        SourceSpecMatcher(SpecParser::FileContext *specParseTree,
                          llvm::Module *irModule);

        // Starts the matching-process
        std::unique_ptr<MatchedSpecifications> matchSpecs();

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
        virtual std::string matchFContract(SrcRange loc) = 0;

        /**
         * Given the location of a specification-block of loop-invariants,
         * returns a pointer to the loop that this specification refers to,
         * and a pointer to the function that the matched loop is contained in.
         * If the spec-block cannot be matched, an exception it thrown.
         *
         */
        virtual std::pair<llvm::Function *, llvm::Loop *>
        matchLoopInvBlock(SrcRange loc) = 0;

        /**
         * Given the location of a block of spec-statements,
         * returns a pointer to the instruction that the specification refer to.
         * If the spec-block cannot be matched, an exception it thrown.
         */
        virtual llvm::Instruction *matchStatementBlock(SrcRange loc) = 0;

    private:
        // Struct in which the matched specifications are collected.
        std::unique_ptr<MatchedSpecifications> matchedSpecs = nullptr;

        // Temporarily stores the pointer to the current function-contract
        // while the contained contract-statements are collected
        std::unique_ptr<sourcespec2ir::MatchedFunctionContract>
            contract = nullptr;

        // Temporary pointer to the current spec-statement-block while the
        // statements are matched.
        std::unique_ptr<sourcespec2ir::MatchedSpecStatementBlock>
            stmntBlock = nullptr;

        // Visitors for function-contracts
        std::any visitSpecContractBlock(
            SpecParser::SpecContractBlockContext *ctx) override;
        std::any visitPureFlag(SpecParser::PureFlagContext *ctx) override;
        std::any visitRequiresClause(
            SpecParser::RequiresClauseContext *ctx) override;
        std::any visitEnsuresClause(
            SpecParser::EnsuresClauseContext *ctx) override;
        // Visitor for loop-invariants
        std::any visitSpecLoopInvariantBlock(
            SpecParser::SpecLoopInvariantBlockContext *ctx) override;
        // Visitor for specification-statement blocks.
        std::any visitSpecStatementBlock(
            SpecParser::SpecStatementBlockContext *ctx) override;
        std::any visitAssertStatement(
            SpecParser::AssertStatementContext *ctx) override;
        std::any visitAssumeStatement(
            SpecParser::AssumeStatementContext *ctx) override;

        // Adds the given statement to the current function-contract
        // (SourceSpecMatcher::contract).
        void addToFContract(ContractStatementType type,
                            SpecParser::SpecExpressionContext *expr, 
                            const SrcRange &loc);

        // Adds a statement to the current block of specification-statements
        void addToStmntBlock(SpecStatementType type,
                             SpecParser::SpecExpressionContext *expr, 
                             const SrcRange &loc);

        /**
         * Retrieving the textual representation of the specification
         * expressions using getText() removes the whitespaces.
         * This method returns the full textual representatoin including
         * whitespaces by using the underlying token-stream.
         */
        std::string getExprAsStr(const SpecParser::SpecExpressionContext &exprCtx);

        /**
         * Extracts the source-location from a given parser-rule context and
         * returns it as a SrcRange. 
         */
        SrcRange getLocFromCtx(const antlr4::ParserRuleContext *ctx);

        /**
         * Returns the location in the source-file where the given expression 
         * starts. 
         */
        SrcLocation getExprStartLoc(
            const SpecParser::SpecExpressionContext &exprCtx);
    };

}

#endif // PALLAS_SOURCESPECMATCHER_H