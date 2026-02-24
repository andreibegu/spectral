#ifndef PALLAS_IRBASEDSOURCESPECMATCHER_H
#define PALLAS_IRBASEDSOURCESPECMATCHER_H

#include "SourceSpecMatcher.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>

#include <string>

namespace sourcespec2ir
{

    /**
     * An implementation of the SourceSpecMatcher that uses the line- and
     * column-information from the debug-information in the LLVM-IR.
     *
     * During the matching-process, only functions from the same
     * file are considered. This is required to previent conflicting
     * line-information when other files are included.
     * The provided srcFilePath is used to determine the file from which the
     * specifications were extracted.
     */
    class IRBasedSourceSpecMatcher : public SourceSpecMatcher
    {

    public:
        IRBasedSourceSpecMatcher(SpecParser::FileContext *specParseTree,
                                 const std::string &srcFilePath,
                                 llvm::Module *irModule,
                                 llvm::FunctionAnalysisManager *fam,
                                 const bool printMatchingInfo,
                                 bool matchExternalOnly, 
                                 std::unique_ptr<SourceSyntaxResolver> syntaxResolver);

    protected:
        llvm::Function *matchFContract(SrcRange loc) override;

        llvm::SmallVector<llvm::Function *> matchFContract(
            const std::string matchingStr, const bool isSrcName,
            SrcRange loc, const bool isGenerated) override;

        std::pair<llvm::Function *, llvm::Loop *> matchLoopInvBlock(
            SrcRange loc) override;

        llvm::Instruction *matchStatementBlock(SrcRange loc) override;

        llvm::CallInst *matchGhostArgBindingBlock(SrcRange loc) override;

    private:
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

        llvm::Instruction *getClosestInstructionAfter(const SrcRange &loc);

        inline bool isArtificial(const llvm::DISubprogram *sProg)
        {
            return sProg->getFlags() & llvm::DINode::FlagArtificial;
        }
    };

}

#endif // PALLAS_IRBASEDSOURCESPECMATCHER_H