#ifndef PALLAS_IRBASEDSOURCESPECMATCHER_H
#define PALLAS_IRBASEDSOURCESPECMATCHER_H

#include "SourceSpecMatcher.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>

namespace sourcespec2ir
{

    /**
     * An implementation of the SourceSpecMatcher that uses the line- and
     * column-information from the debug-information in the LLVM-IR.
     *
     * During the matching-process, only functions from the same
     * file are considered. This is required to previent conflicting
     * line-information when other files are included.
     */
    class IRBasedSourceSpecMatcher : public SourceSpecMatcher
    {

    public:
        IRBasedSourceSpecMatcher(SpecParser::FileContext *specParseTree,
                                 llvm::Module *irModule,
                                 llvm::FunctionAnalysisManager *fam,
                                 const bool printMatchingInfo);

    protected:
        std::string matchFContract(SrcRange loc) override;

        std::pair<llvm::Function *, llvm::Loop *> matchLoopInvBlock(
            SrcRange loc) override;

        llvm::Instruction *matchStatementBlock(SrcRange loc) override;

    private:
        const bool printMatchingInfo;
        llvm::FunctionAnalysisManager *fam;

        /**
         * Returns wether the given function that is represented by the given
         * DISubprogram is defined in the main source-file of the module
         * (as opposed to being defined in an included file).
         */
        bool isFromMainFile(const llvm::DISubprogram &sProg);

        /**
         * Returns the first function above the given line. 
         * Only functions from the main-file are considered
         */
        llvm::Function *getFirstFunctionAbove(const size_t line);
    };

}

#endif // PALLAS_IRBASEDSOURCESPECMATCHER_H