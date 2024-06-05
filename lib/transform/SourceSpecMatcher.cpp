#include "transform/SourceSpecMatcher.h"
#include "utils/SrcLookup.h"


namespace sourcespec2ir
{
    SourceSpecMatcher::SourceSpecMatcher(SpecParser::FileContext *specParseTree, llvm::Module *irModule)
        : specParseTree(specParseTree),
          irModule(irModule)
    {}

    std::unique_ptr<llvm::SmallVector<std::unique_ptr<MatchedFunctionContract>>> SourceSpecMatcher::matchSpecifications()
    {
        matchedSpecifications.reset();
        matchedSpecifications = std::make_unique<llvm::SmallVector<std::unique_ptr<MatchedFunctionContract>>>();
        visit(specParseTree);
        return std::move(matchedSpecifications);
    }

    std::any SourceSpecMatcher::visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx)
    {
        // Get the name of the function to which the contract-block belongs
        auto startLine = ctx->start->getLine();
        auto startCol = ctx->start->getCharPositionInLine();
        auto endLine = ctx->stop->getLine();
        auto endCol = ctx->stop->getCharPositionInLine();
        const auto specFun = getContractBlockContext(startLine, startCol, endLine, endCol);

        // Get the IR-function that the contract belongs to
        auto irCtxFunction = utils::getFunctionBySrcName(*irModule, specFun);

        matchedSpecifications->push_back(std::make_unique<MatchedFunctionContract>(irCtxFunction, ctx));

        return 0;
    }
    
}