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
            llvm::SmallVector<std::string> extContractPaths,
            const utils::OutputConfig &outputConfig,
            const std::string &workingDir,
            bool runMem2Reg);

    protected:
        std::unique_ptr<SourceWrapperGenerator> createWrapperGenerator(
            const llvm::SmallVector<SpecDeclaration> &specDeclarations,
            const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers) override;
            std::unique_ptr<SourceSyntaxResolver> createSyntaxResolver() override;
    };

}

#endif // PALLAS_SWIFTSPECTRANSFORMER_H