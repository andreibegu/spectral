#ifndef PALLAS_DUMMYSPECMATCHER_H
#define PALLAS_DUMMYSPECMATCHER_H

#include "SourceSpecMatcher.h"

namespace sourcespec2ir
{

    /**
     * A dummy implementation of the SourceSpecMatcher that always returns the same string.
     * For testing-purposes only.
     */
    class DummySpecMatcher : public SourceSpecMatcher
    {

    public:
        DummySpecMatcher(SpecParser::FileContext *specParseTree, llvm::Module *irModule, std::string contractDefaultCtxName);

    protected:
        std::string matchFContract(size_t startLine, size_t startCol, size_t endLine, size_t endCol);

    private:
        std::string defaultContractCtx;
    };

}

#endif // PALLAS_DUMMYSPECMATCHER_H