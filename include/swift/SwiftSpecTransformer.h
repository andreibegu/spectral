#ifndef PALLAS_SWIFTSPECTRANSFORMER_H
#define PALLAS_SWIFTSPECTRANSFORMER_H

#include "SourceSpecTransformer.h"

namespace sourcespec2ir
{

    /**
     * Specialization of the SourceSpecTransformer for Swift.
     */
    class SwiftSpecTransformer : public SourceSpecTransformer
    {
    public:
        SwiftSpecTransformer(
            std::string specLibPath,
            std::string srcInPath, std::string outPath,
            const utils::OutputConfig &outputConfig, 
            const std::string &workingDir, 
            bool runMem2Reg);
    protected:
        std::unique_ptr<SourceWrapperGenerator> createWrapperGenerator(
            const llvm::SmallVector<SpecDeclaration> &specDeclarations
        ) override;
    };

}

#endif // PALLAS_SWIFTSPECTRANSFORMER_H