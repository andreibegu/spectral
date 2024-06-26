#ifndef PALLAS_IRBASEDSOURCESPECMATCHER_H
#define PALLAS_IRBASEDSOURCESPECMATCHER_H

#include "SourceSpecMatcher.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>

namespace sourcespec2ir
{

    /**
     * An implementation of the SourceSpecMatcher that uses the line- and column-information
     * from the debug-information in the LLVM-IR.
     */
    class IRBasedSourceSpecMatcher : public SourceSpecMatcher
    {

    public:
        IRBasedSourceSpecMatcher(SpecParser::FileContext *specParseTree,
                                 llvm::Module *irModule, 
                                 llvm::FunctionAnalysisManager *fam, 
                                 const bool printMatchingInfo);

    protected:
        std::string getContractBlockContext(size_t startLine, size_t startCol, size_t endLine, size_t endCol) override;
        std::pair<llvm::Function *, llvm::Loop *> matchLoopInvBlock(size_t startLine, size_t startCol, size_t endLine, size_t endCol) override;

    private:
        const bool printMatchingInfo;
        llvm::FunctionAnalysisManager *fam;
    };

}

#endif // PALLAS_IRBASEDSOURCESPECMATCHER_H