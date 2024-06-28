#include "SourceSpecMatcher.h"
#include "PallasException.h"
#include "SrcLookup.h"

#include <cassert>
#include <stdexcept>

namespace sourcespec2ir
{
    SourceSpecMatcher::SourceSpecMatcher(SpecParser::FileContext *specParseTree, llvm::Module *irModule)
        : specParseTree(specParseTree),
          irModule(irModule),
          matchedFuncContracts(),
          matchedLoopInvariants()
    {
    }

    void SourceSpecMatcher::matchSpecs()
    {
        // Traverse the parse-tree and match all specifications
        visit(specParseTree);
    }

    std::any SourceSpecMatcher::visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx)
    {
        // Safety-check that the state of the object is valid
        if (contract)
            assert("Unexpected state while matching contract");

        // Match contract to function
        auto startLine = ctx->start->getLine();
        auto startCol = ctx->start->getCharPositionInLine();
        auto endLine = ctx->stop->getLine();
        auto endCol = ctx->stop->getCharPositionInLine();
        const auto specFun = getContractBlockContext(startLine, startCol,
                                                     endLine, endCol);
        auto irFunction = utils::getFunctionBySrcName(*irModule, specFun);
        // Create new matched contract
        contract = std::make_unique<MatchedFunctionContract>(irFunction);

        // Add all statements to the contract by traversing the children
        for (auto c : ctx->children)
        {
            visit(c);
        }

        // Add the new contract to the collection of contracts.
        matchedFuncContracts.push_back(std::move(contract));
        return 0;
    }

    std::any SourceSpecMatcher::visitRequiresClause(SpecParser::RequiresClauseContext *ctx)
    {
        addStatementToFContract(ContractStatementType::Requires,
                                ctx->specExpression());
        return 0;
    }

    std::any SourceSpecMatcher::visitEnsuresClause(SpecParser::EnsuresClauseContext *ctx)
    {
        addStatementToFContract(ContractStatementType::Ensures,
                                ctx->specExpression());
        return 0;
    }

    void SourceSpecMatcher::addStatementToFContract(ContractStatementType type,
                                                    SpecParser::SpecExpressionContext *expr)
    {
        if (!contract)
            assert("No contract set while adding statement");
        auto exprStr = getExprAsStr(*expr);
        auto stmnt = std::make_unique<MatchedFContractStatement>(type, exprStr);
        contract->addStatement(std::move(stmnt));
    }

    std::any SourceSpecMatcher::visitSpecLoopInvariantBlock(SpecParser::SpecLoopInvariantBlockContext *ctx)
    {
        // Match the spec-block to the IR
        auto startLine = ctx->start->getLine();
        auto startCol = ctx->start->getCharPositionInLine();
        auto endLine = ctx->stop->getLine();
        auto endCol = ctx->stop->getCharPositionInLine();
        auto [parentFunc, loop] = matchLoopInvBlock(startLine, startCol, endLine, endCol);

        if (parentFunc == nullptr || loop == nullptr)
        {
            // Should never happen, since matchLoopInvBlock should
            // throw the error.
            throw utils::PallasException("Failed to match loop-invariant");
        }

        auto loopID = llvm::dyn_cast_or_null<llvm::MDTuple>(loop->getLoopID());
        if (loopID == nullptr)
        {
            throw utils::PallasException("Failed to get llvm.loop md-node for loop");
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

        matchedLoopInvariants.push_back(std::move(invBlock));
        return 0;
    }

    size_t SourceSpecMatcher::getNumMatchedFuncContracts()
    {
        return matchedFuncContracts.size();
    }

    MatchedFunctionContract *SourceSpecMatcher::getMatchedFuncContract(size_t idx)
    {
        return matchedFuncContracts[idx].get();
    }

    size_t SourceSpecMatcher::getNumMatchedLoopInv()
    {
        return matchedLoopInvariants.size();
    }

    MatchedLoopInvariantBlock *SourceSpecMatcher::getMatchedLoopInv(size_t idx)
    {
        return matchedLoopInvariants[idx].get();
    }

    std::string SourceSpecMatcher::getExprAsStr(const SpecParser::SpecExpressionContext &eCtx)
    {
        auto startToken = eCtx.start;
        auto stopToken = eCtx.stop;
        auto charStream = startToken->getTokenSource()->getInputStream();

        antlr4::misc::Interval i(startToken->getStartIndex(),
                                 stopToken->getStopIndex());
        return charStream->getText(i);
    }

}