#include "SourceSpecMatcher.h"

#include "FileUtils.h"
#include "PallasException.h"
#include "ParsingUtils.h"
#include "SpecLexer.h"

#include <cassert>
#include <optional>

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringExtras.h>

namespace
{
    void printGhostMethodMatchingInfo(const sourcespec2ir::SrcRange &loc,
                                      std::string name, std::string type)
    {
        llvm::outs() << "Found ghost " << type << " " << name << " at "
                     << loc.getStartLine() << "::" << loc.getStartCol()
                     << " - "
                     << loc.getEndLine() << "::" << loc.getEndCol()
                     << "\n";
    }
}

namespace sourcespec2ir
{
    SourceSpecMatcher::SourceSpecMatcher(
        SpecParser::FileContext *specParseTree,
        const std::string file,
        llvm::Module *irModule,
        const bool printMatchingInfo,
        bool matchExternalOnly,
        std::unique_ptr<SourceSyntaxResolver> syntaxResolver)
        : specParseTree(specParseTree),
          file(file),
          md5Checksum(utils::getChecksumForFile(file)),
          irModule(irModule),
          printMatchingInfo(printMatchingInfo),
          matchExternalOnly(matchExternalOnly),
          syntaxResolver(std::move(syntaxResolver))
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
            irFunctions = matchFContract(matchingStr, isSrcMapping, loc,
                                         type == Generated);
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

    std::any SourceSpecMatcher::visitGhostDefBlock(
        SpecParser::GhostDefBlockContext *ctx)
    {
        // Safety-check that the state of the object is valid
        assert(ghostContract == nullptr);

        // Construct a new ghost function:
        auto defCtx = ctx->ghostDef();
        auto srcLoc = utils::getLocFromCtx(ctx, file, md5Checksum);
        auto defLoc = utils::getGhostImplStartLoc(*defCtx->impl, file,
                                                  md5Checksum);
        bool isFunction = defCtx->type->getType() == SpecLexer::FUNCTION;
        auto definition = utils::getCtxAsStr(*defCtx->impl);
        std::string name = getSyntaxResolver().getGhostMethodName(definition);
        auto ghostMethod = std::make_unique<GhostMethod>(
            isFunction, name, definition, srcLoc, defLoc);

        if (printMatchingInfo)
            printGhostMethodMatchingInfo(
                srcLoc, name, defCtx->type->getText());

        auto *gMethodPtr = ghostMethod.get();
        matchedSpecs->ghostMethods.push_back(std::move(ghostMethod));

        // Construct contract for ghost function:
        auto contractCtx = defCtx->contr;
        auto contractLoc = utils::getLocFromCtx(contractCtx, file, md5Checksum);
        auto sig = getSigForGhostMethod(definition);
        ghostContract = std::make_unique<MatchedGhostContract>(
            gMethodPtr, contractLoc, sig);

        // Add all statements to the contract by traversing the children
        for (auto c : contractCtx->specContractClause())
            visit(c);
        matchedSpecs->matchedGhostContracts.push_back(std::move(ghostContract));

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
        sig.returnTy = std::nullopt;
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

    std::any SourceSpecMatcher::visitGivenDefClause(
        SpecParser::GivenDefClauseContext *ctx)
    {
        if (contract == nullptr)
            throw utils::PallasException(
                "Given-clause only allowed in contracts.");

        auto defExpr = ctx->specExpression();
        auto defStr = utils::getCtxAsStr(*defExpr);
        GhostArgumentDef argDef(defStr, getSyntaxResolver().getArgName(defStr),
                                getLocFromCtx(ctx), getExprStartLoc(*defExpr));
        contract->givenArgs.push_back(argDef);
        return 0;
    }

    std::any SourceSpecMatcher::visitYieldsDefClause(
        SpecParser::YieldsDefClauseContext *ctx)
    {
        if (contract == nullptr)
            throw utils::PallasException(
                "Yields-clause only allowed in contracts.");

        auto defExpr = ctx->specExpression();
        auto defStr = utils::getCtxAsStr(*defExpr);
        GhostArgumentDef argDef(defStr, getSyntaxResolver().getArgName(defStr),
                                getLocFromCtx(ctx), getExprStartLoc(*defExpr));
        contract->yieldsArgs.push_back(argDef);
        return 0;
    }

    void SourceSpecMatcher::addToFContract(
        ContractStatementType type, SpecParser::SpecExpressionContext *expr,
        const SrcRange &loc)
    {
        auto exprStr = utils::getCtxAsStr(*expr);
        if (contract && !ghostContract)
        {
            auto extContr = contract->type == External;
            auto stmnt = std::make_unique<MatchedFContractStatement>(
                type, exprStr, loc, getExprStartLoc(*expr), extContr);
            contract->addStatement(std::move(stmnt));
        }
        else if (ghostContract && !contract)
        {
            auto stmnt = std::make_unique<MatchedFContractStatement>(
                type, exprStr, loc, getExprStartLoc(*expr), false);
            ghostContract->addStatement(std::move(stmnt));
        }
        else
            throw utils::PallasException("Failed to add contract statement!");
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

        // Match specifications to instruction
        auto loc = getLocFromCtx(ctx);
        const auto instr = matchStatementBlock(loc);
        assert(instr != nullptr);

        // Create new matched spec-block
        stmntBlock = std::make_unique<MatchedSpecStatementBlock>(instr, loc);

        // Add all statements by traversing the children
        for (auto c : ctx->children)
            visit(c);

        // Add to the collection of matched specifications.
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

    std::any SourceSpecMatcher::visitFoldStatement(
        SpecParser::FoldStatementContext *ctx)
    {
        addToStmntBlock(Fold, ctx->specExpression(), getLocFromCtx(ctx));
        return 0;
    }

    std::any SourceSpecMatcher::visitUnfoldStatement(
        SpecParser::UnfoldStatementContext *ctx)
    {
        addToStmntBlock(Unfold, ctx->specExpression(), getLocFromCtx(ctx));
        return 0;
    }

    std::any SourceSpecMatcher::visitGhostAssignment(
        SpecParser::GhostAssignmentContext *ctx)
    {
        auto gVarName = std::make_optional(utils::getCtxAsStr(*ctx->gVar));
        addToStmntBlock(
            GhostAssign, ctx->specExpression(), getLocFromCtx(ctx), gVarName);
        return 0;
    }

    void SourceSpecMatcher::addToStmntBlock(
        SpecStatementType type, SpecParser::SpecExpressionContext *expr,
        const SrcRange &loc, std::optional<std::string> gVarName)
    {
        if (!stmntBlock)
            assert("No parent set while adding spec-statement");
        auto exprStr = utils::getCtxAsStr(*expr);
        auto stmnt = std::make_unique<MatchedSpecStatement>(
            type, exprStr, loc, getExprStartLoc(*expr), gVarName);
        stmntBlock->addStatement(std::move(stmnt));
    }

    std::any SourceSpecMatcher::visitGivenBindingBlock(
        SpecParser::GivenBindingBlockContext *ctx)
    {
        assert(ghostArgBindingBlock == nullptr);

        // Match to call-instruction
        auto loc = getLocFromCtx(ctx);
        const auto callInstr = matchGhostArgBindingBlock(loc);
        assert(callInstr != nullptr);

        ghostArgBindingBlock = std::make_unique<MatchedGhostArgBindingBlock>(
            callInstr, loc);

        // Add bindings:
        for (auto c : ctx->children)
            visit(c);

        matchedSpecs->matchedGivenBindings.push_back(
            std::move(ghostArgBindingBlock));
        return 0;
    }

    std::any SourceSpecMatcher::visitGivenArgBinding(
        SpecParser::GivenArgBindingContext *ctx)
    {
        assert(ghostArgBindingBlock != nullptr);

        auto argName = utils::getCtxAsStr(*ctx->gArg);
        auto exprStr = utils::getCtxAsStr(*ctx->expr);
        auto loc = getLocFromCtx(ctx);
        auto exprStartLoc = getExprStartLoc(*ctx->expr);
        ghostArgBindingBlock->addBinding(
            MatchedGhostVarBinding(argName, exprStr, loc, exprStartLoc));
        return 0;
    }

    std::any SourceSpecMatcher::visitYieldsBindingBlock(
        SpecParser::YieldsBindingBlockContext *ctx)
    {
        assert(ghostArgBindingBlock == nullptr);

        // Match to call-instruction
        auto loc = getLocFromCtx(ctx);
        const auto callInstr = matchGhostArgBindingBlock(loc);
        assert(callInstr != nullptr);

        ghostArgBindingBlock = std::make_unique<MatchedGhostArgBindingBlock>(
            callInstr, loc);

        // Add bindings:
        for (auto c : ctx->children)
            visit(c);

        matchedSpecs->matchedYieldsBindings.push_back(
            std::move(ghostArgBindingBlock));
        return 0;
    }

    std::any SourceSpecMatcher::visitYieldsArgBinding(
        SpecParser::YieldsArgBindingContext *ctx)
    {
        assert(ghostArgBindingBlock != nullptr);

        auto argName = utils::getCtxAsStr(*ctx->yArg);
        auto exprStr = utils::getCtxAsStr(*ctx->expr);
        auto loc = getLocFromCtx(ctx);
        auto exprStartLoc = loc.startLoc;
        ghostArgBindingBlock->addBinding(
            MatchedGhostVarBinding(argName, exprStr, loc, exprStartLoc));
        return 0;
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

    WrapperSignature SourceSpecMatcher::getSigForGhostMethod(
        llvm::StringRef impl)
    {
        // Currently the wrapper can only access the arguments of the function.
        // If global variables are also suppoed to be supported, this needs to
        // be extended.
        WrapperSignature sig;
        sig.parentRetTy = getSyntaxResolver().getGhostMethodRetTy(impl);
        sig.returnTy = std::nullopt;
        sig.args = getSyntaxResolver().getGhostMethodArgs(impl);
        return sig;
    }

    SourceSyntaxResolver &SourceSpecMatcher::getSyntaxResolver()
    {
        assert(syntaxResolver != nullptr);
        return *syntaxResolver;
    }
}