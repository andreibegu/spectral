#ifndef PALLAS_DUMMYSPECCONTEXTPROVIDER_H
#define PALLAS_DUMMYSPECCONTEXTPROVIDER_H

#include "SourceSpecContextProvider.h"

namespace sourcespec2ir 
{

    /**
     * A dummy implementation of the SourceSpecContextProvider that always returns the same string.
     * For testing-purposes only.  
     */
    class DummySourceSpecContextProvider : public SourceSpecContextProvider {
    private:
        std::string defaultContractCtx;

    public:
        DummySourceSpecContextProvider(std::string contractDefaultCtxName);

        std::string getContractBlockContext(size_t startLine, size_t startCol, size_t endLine, size_t endCol);
    
    };

}

#endif // PALLAS_DUMMYSPECCONTEXTPROVIDER_H