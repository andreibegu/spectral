
#include "transform/DummySourceSpecContextProvider.h"

namespace sourcespec2ir
{

    DummySourceSpecContextProvider::DummySourceSpecContextProvider(std::string contractDefaultCtxName)
        : defaultContractCtx(contractDefaultCtxName)
    {
        // Nothing to see here ...
    }

    std::string DummySourceSpecContextProvider::getContractBlockContext(size_t startLine, size_t startCol, size_t endLine, size_t endCol)
    {
        return defaultContractCtx;
    }
}