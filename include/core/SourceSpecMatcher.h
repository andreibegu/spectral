#ifndef PALLAS_SOURCESPECMATCHER_H
#define PALLAS_SOURCESPECMATCHER_H

#include "MatchedSourceSpec.h"
#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"
#include "wrapper/SrcLocation.h"

#include <optional>
#include <memory>
#include <string>
#include <utility>

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
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
                          const std::string file,
                          llvm::Module *irModule, 
                          const bool printMatchingInfo,
                          bool matchExternalOnly);

        virtual ~SourceSpecMatcher() = default;

        // Starts the matching-process
        std::unique_ptr<MatchedSpecifications> matchSpecs();

    protected:
        SpecParser::FileContext *specParseTree;
        const std::string file;
        std::string md5Checksum;
        llvm::Module *irModule;
        const bool printMatchingInfo;
        bool matchExternalOnly;

        /**
         * Matches a function-contract based on a provided matching-string.
         * Given the matching-string this method determines the functions in the
         * module whose names match to the provided matching-string.
         * The flag 'isSrcName' determines if the name should be interpreted as
         *  source- or ir-name.
         *
         * The default-implementation only supports source-level matching
         * strings and interprets them as a regular expressions that match on
         * the function's name.
         */
        virtual llvm::SmallVector<llvm::Function *> matchFContract(
            const std::string matchingStr, const bool isSrcName,
            SrcRange loc, const bool isGenerated) = 0;

        /**
         * Given the location of a contract-block in the source-file,
         * this method returns the name of the function that the specification
         * refers to.
         */
        virtual llvm::Function *matchFContract(SrcRange loc) = 0;

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

        /**
         * Returns a SrcRang for the given ParserRuleContext.
         * Sets file and checksum to the file that is opened by this matcher.
         */
        SrcRange getLocFromCtx(const antlr4::ParserRuleContext *ctx);

        /**
         * Returns the location in the file opened by this matcher
         * where the given expression starts.
         */
        SrcLocation getExprStartLoc(
            const SpecParser::SpecExpressionContext &exprCtx);

        /**
         * Thive the string-representation of a ghost method, returns the name 
         * of the ghost method as a string. 
         * The default-implementation looks for the first occurence of '(' and 
         * returns the whitespace free character sequence in front of it. 
         * If the impl. does not contain a '(' character, an empty string is 
         * returned. 
         * I.e. for 'int foo(...' it would return 'foo'. 
         * 
         * Can be overridden if the default-implementation is not suitable.
         */
        virtual std::string getGhostMethodName(llvm::StringRef impl);

        /**
         * Given a string that represents a language-specific definition of a 
         * ghost method or function, return the arguments of the method.
         * The default implementation assumes a C-style syntax 
         * (i.e. function arguments are enclosed in '(' and ')' and separated 
         * by ','). 
         * E.g. for 'int foo(int a, int b) { ...' it would return 
         * 'int a' and 'int b'.
         * 
         * If the default implementation is not suitable for a particular
         * language, it can be overridden. 
         */
        virtual llvm::SmallVector<std::string> getGhostMethodArgs(
            llvm::StringRef impl);

        /**
         * Given a string that represents a language-specific definition of a 
         * ghost method / function, return the method's return type as a string.
         * The default implementation assumes a C-style syntax 
         * 
         * If the default implementation is not suitable for a particular
         * language, it can be overridden. 
         */
        virtual std::string getGhostMethodRetTy(llvm::StringRef impl);

        /**
         * Given a string that represents a language-specific definition of a 
         * ghost method or function, return a WrapperSignature for this 
         * function. 
         * Uses getGhostMethodArgs and getGhostMethodRetTy. 
         */
        virtual WrapperSignature getSigForGhostMethod(llvm::StringRef impl);

    private:
        // Struct in which the matched specifications are collected.
        std::unique_ptr<MatchedSpecifications> matchedSpecs = nullptr;

        // Temporarily stores the pointer to the current function-contract
        // while the contained contract-statements are collected
        std::unique_ptr<sourcespec2ir::MatchedFunctionContract>
            contract = nullptr;
        
        std::unique_ptr<sourcespec2ir::MatchedGhostContract>
            ghostContract = nullptr;
        

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
        std::any visitGivenDefClause(
            SpecParser::GivenDefClauseContext *ctx) override;
        std::any visitYieldsDefClause(
            SpecParser::YieldsDefClauseContext *ctx) override;

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
        std::any visitFoldStatement(
            SpecParser::FoldStatementContext *ctx) override;
        std::any visitUnfoldStatement(
            SpecParser::UnfoldStatementContext *ctx) override;

        // Visitor for ghost methods
        std::any visitGhostDefBlock(
            SpecParser::GhostDefBlockContext *ctx) override;

        // Adds the given statement to the current function-contract
        // (SourceSpecMatcher::contract).
        void addToFContract(ContractStatementType type,
                            SpecParser::SpecExpressionContext *expr,
                            const SrcRange &loc);

        // Adds a statement to the current block of specification-statements
        void addToStmntBlock(SpecStatementType type,
                             SpecParser::SpecExpressionContext *expr,
                             const SrcRange &loc);

        WrapperSignature getWrapperSignature(
            SpecParser::WrapperSigClauseContext &ctx);
    };

}

#endif // PALLAS_SOURCESPECMATCHER_H