
#include "core/DummySpecMatcher.h"

namespace sourcespec2ir
{

    DummySpecMatcher::DummySpecMatcher(SpecParser::FileContext *specParseTree, llvm::Module *irModule, std::string contractDefaultCtxName)
        : SourceSpecMatcher(specParseTree, irModule),
          defaultContractCtx(contractDefaultCtxName)
    {
    }

    std::string DummySpecMatcher::getContractBlockContext(size_t startLine, size_t startCol, size_t endLine, size_t endCol)
    {
        return defaultContractCtx;
    }
}