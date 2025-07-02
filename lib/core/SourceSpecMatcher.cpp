#include "SourceSpecMatcher.h"
#include "PallasException.h"
#include "ParsingUtils.h"
#include "FileUtils.h"

#include <cassert>
#include <optional>

namespace sourcespec2ir
{
    SourceSpecMatcher::SourceSpecMatcher(SpecParser::FileContext *specParseTree,
                                         const std::string file,
                                         llvm::Module *irModule,
                                         bool matchExternalOnly)
        : specParseTree(specParseTree),
          file(file),
          md5Checksum(utils::getChecksumForFile(file)),
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

        auto loc = getLocFromCtx(ctx);
        auto type = getContractType(*ctx);
        auto *mappingClause = ctx->specMappingClause();
        bool hasMappingClause = mappingClause != nullptr;
        bool hasSigClause = hasMappingClause &&
                            mappingClause->wrapperSigClause() != nullptr;
        bool isAssumed = hasMappingClause &&
                         mappingClause->ASSUMED() != nullptr;

        if (matchExternalOnly && type == Normal)
            throw utils::PallasException(
                "Encountered non-external contract in contract-file.");

        if (type != External && hasSigClause)
            throw utils::PallasException("Specification of wrapper-signature "
                                         "is only allowed for external contracts.");

        llvm::SmallVector<llvm::Function *> irFunctions;
        if (type == External || type == Generated)
        {
            // Match using matching-string
            bool isSrcMapping = mappingClause->SRC() != nullptr;
            auto matchingStr = utils::getCtxAsStr(*mappingClause->specExpression());
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

        std::optional<WrapperSignature> sig = std::nullopt;
        if (hasSigClause)
            sig = getWrapperSignature(*mappingClause->wrapperSigClause());

        for (auto *irFunction : irFunctions)
        {
            // Create new matched contract
            contract = std::make_unique<MatchedFunctionContract>(
                irFunction, loc, type);
            contract->isAssumed = isAssumed;
            contract->wrapperSignature = sig;

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

    WrapperSignature SourceSpecMatcher::getWrapperSignature(
        SpecParser::WrapperSigClauseContext &ctx)
    {
        WrapperSignature sig;
        // Use visitor to turn arg-list into vector.
        auto *argList = ctx.wrapperArgClause()->wrapperArgList();
        while (argList != nullptr)
        {
            sig.addArg(utils::getCtxAsStr(*argList->argExpr()));
            argList = argList->wrapperArgList();
        }

        // Extract return-type
        auto *retClause = ctx.wrapperRetClause();
        sig.parentRetTy = utils::getCtxAsStr(*retClause->specExpression());
        return sig;
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
        auto exprStr = utils::getCtxAsStr(*expr);
        auto extContr = contract->type == External;
        auto stmnt = std::make_unique<MatchedFContractStatement>(
            type, exprStr, loc, getExprStartLoc(*expr), extContr);
        contract->addStatement(std::move(stmnt));
    }

    std::any SourceSpecMatcher::visitSpecLoopInvariantBlock(
        SpecParser::SpecLoopInvariantBlockContext *ctx)
    {
        if (matchExternalOnly)
            throw utils::PallasException(
                "Encountered loop invariant in contract-file.");

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
            throw utils::PallasException("Failed to get llvm.loop md-node");

        auto loopHeader = loop->getHeader();
        auto invBlock = std::make_unique<MatchedLoopInvariantBlock>(
            parentFunc, loopID, loopHeader, loc);

        // Collect the individual invariants from the invariant-block
        for (auto *inv : ctx->specLoopInvariant())
        {
            auto stringRepr = utils::getCtxAsStr(*inv->specExpression());
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

        if (matchExternalOnly)
            throw utils::PallasException(
                "Encountered statement-block in contract-file.");

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
        auto exprStr = utils::getCtxAsStr(*expr);
        auto stmnt = std::make_unique<MatchedSpecStatement>(
            type, exprStr, loc, getExprStartLoc(*expr));
        stmntBlock->addStatement(std::move(stmnt));
    }

    SrcRange SourceSpecMatcher::getLocFromCtx(
        const antlr4::ParserRuleContext *ctx)
    {
        return utils::getLocFromCtx(ctx, file, md5Checksum);
    }

    SrcLocation SourceSpecMatcher::getExprStartLoc(
        const SpecParser::SpecExpressionContext &exprCtx)
    {
        return utils::getExprStartLoc(exprCtx, file, md5Checksum);
    }

}