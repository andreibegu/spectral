#include "SourceSpecMatcher.h"
#include "PallasException.h"
#include "SrcLookup.h"

#include <cassert>
#include <stdexcept>

namespace sourcespec2ir
{
    SourceSpecMatcher::SourceSpecMatcher(SpecParser::FileContext *specParseTree,
                                         llvm::Module *irModule)
        : specParseTree(specParseTree),
          irModule(irModule)
    {
    }

    std::unique_ptr<MatchedSpecifications> SourceSpecMatcher::matchSpecs()
    {
        matchedSpecs = std::make_unique<MatchedSpecifications>();

        // Traverse the parse-tree and match all specifications
        visit(specParseTree);

        return std::move(matchedSpecs);
    }

    std::any SourceSpecMatcher::visitSpecContractBlock(
        SpecParser::SpecContractBlockContext *ctx)
    {
        // Safety-check that the state of the object is valid
        assert(contract == nullptr);

        // Match contract to function
        auto startLine = ctx->start->getLine();
        auto startCol = ctx->start->getCharPositionInLine();
        auto endLine = ctx->stop->getLine();
        auto endCol = ctx->stop->getCharPositionInLine();
        const auto specFun = matchFContract(startLine, startCol,
                                            endLine, endCol);
        auto irFunction = utils::getFunctionBySrcName(*irModule, specFun);
        assert(irFunction != nullptr);

        // Create new matched contract
        contract = std::make_unique<MatchedFunctionContract>(irFunction);

        // Add all statements to the contract by traversing the children
        for (auto c : ctx->children)
            visit(c);

        // Add the new contract to the collection of contracts.
        matchedSpecs->matchedFContracts.push_back(std::move(contract));
        return 0;
    }

    std::any SourceSpecMatcher::visitRequiresClause(
        SpecParser::RequiresClauseContext *ctx)
    {
        addToFContract(Requires, ctx->specExpression());
        return 0;
    }

    std::any SourceSpecMatcher::visitEnsuresClause(
        SpecParser::EnsuresClauseContext *ctx)
    {
        addToFContract(Ensures, ctx->specExpression());
        return 0;
    }

    void SourceSpecMatcher::addToFContract(
        ContractStatementType type, SpecParser::SpecExpressionContext *expr)
    {
        if (!contract)
            assert("No contract set while adding statement");
        auto exprStr = getExprAsStr(*expr);
        auto stmnt = std::make_unique<MatchedFContractStatement>(type, exprStr);
        contract->addStatement(std::move(stmnt));
    }

    std::any SourceSpecMatcher::visitSpecLoopInvariantBlock(
        SpecParser::SpecLoopInvariantBlockContext *ctx)
    {
        // Match the spec-block to the IR
        auto startLine = ctx->start->getLine();
        auto startCol = ctx->start->getCharPositionInLine();
        auto endLine = ctx->stop->getLine();
        auto endCol = ctx->stop->getCharPositionInLine();
        auto [parentFunc, loop] = matchLoopInvBlock(
            startLine, startCol, endLine, endCol);

        if (parentFunc == nullptr || loop == nullptr)
        {
            // Should never happen, since matchLoopInvBlock should
            // throw the error.
            throw utils::PallasException("Failed to match loop-invariant");
        }

        auto loopID = llvm::dyn_cast_or_null<llvm::MDTuple>(loop->getLoopID());
        if (loopID == nullptr)
        {
            throw utils::PallasException("Failed to get llvm.loop md-node");
        }
        auto loopHeader = loop->getHeader();
        auto invBlock = std::make_unique<MatchedLoopInvariantBlock>(
            parentFunc, loopID, loopHeader);

        // Collect the individual invariants from the invariant-block
        for (auto *inv : ctx->specLoopInvariant())
        {
            auto stringRepr = getExprAsStr(*inv->specExpression());
            invBlock->addInvariant(
                std::make_unique<MatchedLoopInvariant>(stringRepr));
        }

        matchedSpecs->matchedLoopInvariants.push_back(std::move(invBlock));
        return 0;
    }

    std::any SourceSpecMatcher::visitSpecStatementBlock(
        SpecParser::SpecStatementBlockContext *ctx)
    {
        // Safety-check that the state of the object is valid
        assert(stmntBlock == nullptr);

        // Match contract to instruction
        auto startLine = ctx->start->getLine();
        auto startCol = ctx->start->getCharPositionInLine();
        auto endLine = ctx->stop->getLine();
        auto endCol = ctx->stop->getCharPositionInLine();
        const auto instr = matchStatementBlock(startLine, startCol,
                                               endLine, endCol);
        assert(instr != nullptr);

        // Create new matched contract-block
        stmntBlock = std::make_unique<MatchedSpecStatementBlock>(instr);

        // Add all statements to the contract by traversing the children
        for (auto c : ctx->children)
            visit(c);

        // Add the new contract to the collection of contracts.
        matchedSpecs->matchedStmntBlocks.push_back(std::move(stmntBlock));
        return 0;
    }

    std::any SourceSpecMatcher::visitAssertStatement(
        SpecParser::AssertStatementContext *ctx)
    {
        addToStmntBlock(Assert, ctx->specExpression());
        return 0;
    }

    std::any SourceSpecMatcher::visitAssumeStatement(
        SpecParser::AssumeStatementContext *ctx)
    {
        addToStmntBlock(Assume, ctx->specExpression());
        return 0;
    }

    void SourceSpecMatcher::addToStmntBlock(
        SpecStatementType type, SpecParser::SpecExpressionContext *expr)
    {
        if (!stmntBlock)
            assert("No parent set while adding spec-statement");
        auto exprStr = getExprAsStr(*expr);
        auto stmnt = std::make_unique<MatchedSpecStatement>(type, exprStr);
        stmntBlock->addStatement(std::move(stmnt));
    }

    std::string SourceSpecMatcher::getExprAsStr(
        const SpecParser::SpecExpressionContext &eCtx)
    {
        auto startToken = eCtx.start;
        auto stopToken = eCtx.stop;
        auto charStream = startToken->getTokenSource()->getInputStream();

        antlr4::misc::Interval i(startToken->getStartIndex(),
                                 stopToken->getStopIndex());
        return charStream->getText(i);
    }

}