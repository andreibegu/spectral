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
        auto loc = getLocFromCtx(ctx);
        const auto specFun = matchFContract(loc);
        auto irFunction = utils::getFunctionBySrcName(*irModule, specFun);
        assert(irFunction != nullptr);

        // Create new matched contract
        contract = std::make_unique<MatchedFunctionContract>(irFunction, loc);

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
        addToFContract(Requires, ctx->specExpression(), getLocFromCtx(ctx));
        return 0;
    }

    std::any SourceSpecMatcher::visitEnsuresClause(
        SpecParser::EnsuresClauseContext *ctx)
    {
        addToFContract(Ensures, ctx->specExpression(), getLocFromCtx(ctx));
        return 0;
    }

    void SourceSpecMatcher::addToFContract(
        ContractStatementType type, SpecParser::SpecExpressionContext *expr,
        const SrcRange &loc)
    {
        if (!contract)
            assert("No contract set while adding statement");
        auto exprStr = getExprAsStr(*expr);
        auto stmnt = std::make_unique<MatchedFContractStatement>(
            type, exprStr, loc, getExprStartLoc(*expr));
        contract->addStatement(std::move(stmnt));
    }

    std::any SourceSpecMatcher::visitSpecLoopInvariantBlock(
        SpecParser::SpecLoopInvariantBlockContext *ctx)
    {
        // Match the spec-block to the IR
        auto loc = getLocFromCtx(ctx);
        auto [parentFunc, loop] = matchLoopInvBlock(loc);

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
            parentFunc, loopID, loopHeader, loc);

        // Collect the individual invariants from the invariant-block
        for (auto *inv : ctx->specLoopInvariant())
        {
            auto stringRepr = getExprAsStr(*inv->specExpression());
            auto exprLoc = getExprStartLoc(*inv->specExpression());
            auto invLoc = getLocFromCtx(inv);
            invBlock->addInvariant(
                std::make_unique<MatchedLoopInvariant>(stringRepr, invLoc, exprLoc));
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
        auto loc = getLocFromCtx(ctx);
        const auto instr = matchStatementBlock(loc);
        assert(instr != nullptr);

        // Create new matched contract-block
        stmntBlock = std::make_unique<MatchedSpecStatementBlock>(instr, loc);

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
        addToStmntBlock(Assert, ctx->specExpression(), getLocFromCtx(ctx));
        return 0;
    }

    std::any SourceSpecMatcher::visitAssumeStatement(
        SpecParser::AssumeStatementContext *ctx)
    {
        addToStmntBlock(Assume, ctx->specExpression(), getLocFromCtx(ctx));
        return 0;
    }

    void SourceSpecMatcher::addToStmntBlock(
        SpecStatementType type, SpecParser::SpecExpressionContext *expr,
        const SrcRange &loc)
    {
        if (!stmntBlock)
            assert("No parent set while adding spec-statement");
        auto exprStr = getExprAsStr(*expr);
        auto stmnt = std::make_unique<MatchedSpecStatement>(
            type, exprStr, loc, getExprStartLoc(*expr));
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

    SrcRange SourceSpecMatcher::getLocFromCtx(const antlr4::ParserRuleContext *ctx)
    {
        // We change the column-numbering to start with 1 to be consistent with 
        // the line-info of LLVM
        return SrcRange(ctx->start->getLine(), ctx->start->getCharPositionInLine() + 1,
                        ctx->stop->getLine(), ctx->stop->getCharPositionInLine() + 1);
    }

    SrcLocation SourceSpecMatcher::getExprStartLoc(
        const SpecParser::SpecExpressionContext &exprCtx)
    {
        return SrcLocation(exprCtx.start->getLine(),
                           exprCtx.start->getCharPositionInLine());
    }

}