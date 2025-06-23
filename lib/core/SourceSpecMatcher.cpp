#include "SourceSpecMatcher.h"
#include "PallasException.h"
#include "ParsingUtils.h"

#include <cassert>

namespace sourcespec2ir
{
    SourceSpecMatcher::SourceSpecMatcher(SpecParser::FileContext *specParseTree,
                                         const std::string file,
                                         llvm::Module *irModule,
                                         bool matchExternalOnly)
        : specParseTree(specParseTree),
          file(file),
          irModule(irModule),
          matchExternalOnly(matchExternalOnly)
    {
    }

    std::unique_ptr<MatchedSpecifications> SourceSpecMatcher::matchSpecs()
    {
        matchedSpecs = std::make_unique<MatchedSpecifications>();
        // Traverse the parse-tree and match all specifications
        visit(specParseTree);
        return std::move(matchedSpecs);
    }

    namespace
    {
        ContractType getContractType(SpecParser::SpecContractBlockContext &ctx)
        {
            if (auto *mappingClause = ctx.specMappingClause())
            {
                if (mappingClause->EXTERNAL() != nullptr)
                    return External;
                else if (mappingClause->GENERATED() != nullptr)
                    return Generated;
                else
                    throw utils::PallasException("Invalid contract type!");
            }
            return Normal;
        }
    }

    std::any SourceSpecMatcher::visitSpecContractBlock(
        SpecParser::SpecContractBlockContext *ctx)
    {
        // Safety-check that the state of the object is valid
        assert(contract == nullptr);

        auto loc = utils::getLocFromCtx(ctx, file);
        auto type = getContractType(*ctx);
        bool hasMappingClause = ctx->specMappingClause() != nullptr;
        bool isAssumed = hasMappingClause &&
                         ctx->specMappingClause()->ASSUMED() != nullptr;

        if (matchExternalOnly && type == Normal)
            throw utils::PallasException(
                "Encountered non-external contract in contract-file.");

        if (isAssumed)
            throw utils::PallasException(
                "Assumed is currently not supported for external contracts.");

        llvm::SmallVector<llvm::Function *> irFunctions;
        if (type == External || type == Generated)
        {
            // Match using matching-string
            auto *mappingClause = ctx->specMappingClause();
            bool isSrcMapping = mappingClause->SRC() != nullptr;
            auto matchingStr = utils::getExprAsStr(*mappingClause->specExpression());
            irFunctions = matchFContract(matchingStr, isSrcMapping, loc);
        }
        else
        {
            // Match based on src-location
            auto *irFunction = matchFContract(loc);
            if (irFunction != nullptr)
                irFunctions.push_back(irFunction);
        }

        if (irFunctions.empty() && !hasMappingClause)
            throw utils::PallasException("Failed to match contract to function.");

        for (auto *irFunction : irFunctions)
        {
            // Create new matched contract
            contract = std::make_unique<MatchedFunctionContract>(
                irFunction, loc, type);
            contract->isAssumed = isAssumed;

            // Add all statements to the contract by traversing the children
            for (auto c : ctx->children)
                visit(c);

            matchedSpecs->matchedFContracts.push_back(std::move(contract));
        }

        return 0;
    }

    std::any SourceSpecMatcher::visitPureFlag(SpecParser::PureFlagContext *ctx)
    {
        // Set pure-flag of contract to true
        if (!contract)
            throw utils::PallasException("Encountered pure-flag without contract.");
        contract->isPure = true;
        return 0;
    }

    std::any SourceSpecMatcher::visitRequiresClause(
        SpecParser::RequiresClauseContext *ctx)
    {
        addToFContract(Requires, ctx->specExpression(),
                       utils::getLocFromCtx(ctx, file));
        return 0;
    }

    std::any SourceSpecMatcher::visitEnsuresClause(
        SpecParser::EnsuresClauseContext *ctx)
    {
        addToFContract(Ensures, ctx->specExpression(),
                       utils::getLocFromCtx(ctx, file));
        return 0;
    }

    void SourceSpecMatcher::addToFContract(
        ContractStatementType type, SpecParser::SpecExpressionContext *expr,
        const SrcRange &loc)
    {
        if (!contract)
            assert("No contract set while adding statement");
        auto exprStr = utils::getExprAsStr(*expr);
        auto stmnt = std::make_unique<MatchedFContractStatement>(
            type, exprStr, loc, utils::getExprStartLoc(*expr, file));
        contract->addStatement(std::move(stmnt));
    }

    std::any SourceSpecMatcher::visitSpecLoopInvariantBlock(
        SpecParser::SpecLoopInvariantBlockContext *ctx)
    {
        if (matchExternalOnly)
            throw utils::PallasException(
                "Encountered loop invariant in contract-file.");

        // Match the spec-block to the IR
        auto loc = utils::getLocFromCtx(ctx, file);
        auto [parentFunc, loop] = matchLoopInvBlock(loc);

        if (parentFunc == nullptr || loop == nullptr)
        {
            // Should never happen, since matchLoopInvBlock should
            // throw the error.
            throw utils::PallasException("Failed to match loop-invariant");
        }

        auto loopID = llvm::dyn_cast_or_null<llvm::MDTuple>(loop->getLoopID());
        if (loopID == nullptr)
            throw utils::PallasException("Failed to get llvm.loop md-node");

        auto loopHeader = loop->getHeader();
        auto invBlock = std::make_unique<MatchedLoopInvariantBlock>(
            parentFunc, loopID, loopHeader, loc);

        // Collect the individual invariants from the invariant-block
        for (auto *inv : ctx->specLoopInvariant())
        {
            auto stringRepr = utils::getExprAsStr(*inv->specExpression());
            auto exprLoc = utils::getExprStartLoc(*inv->specExpression(), file);
            auto invLoc = utils::getLocFromCtx(inv, file);
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

        if (matchExternalOnly)
            throw utils::PallasException(
                "Encountered statement-block in contract-file.");

        // Match contract to instruction
        auto loc = utils::getLocFromCtx(ctx, file);
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
        addToStmntBlock(Assert, ctx->specExpression(),
                        utils::getLocFromCtx(ctx, file));
        return 0;
    }

    std::any SourceSpecMatcher::visitAssumeStatement(
        SpecParser::AssumeStatementContext *ctx)
    {
        addToStmntBlock(Assume, ctx->specExpression(),
                        utils::getLocFromCtx(ctx, file));
        return 0;
    }

    void SourceSpecMatcher::addToStmntBlock(
        SpecStatementType type, SpecParser::SpecExpressionContext *expr,
        const SrcRange &loc)
    {
        if (!stmntBlock)
            assert("No parent set while adding spec-statement");
        auto exprStr = utils::getExprAsStr(*expr);
        auto stmnt = std::make_unique<MatchedSpecStatement>(
            type, exprStr, loc, utils::getExprStartLoc(*expr, file));
        stmntBlock->addStatement(std::move(stmnt));
    }
}