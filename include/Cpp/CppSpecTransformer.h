#ifndef PALLAS_CPPSPECTRANSFORMER_H
#define PALLAS_CPPSPECTRANSFORMER_H

#include "SourceSpecTransformer.h"

namespace sourcespec2ir
{

    /**
     * Specialization of the SourceSpecTransformer for C++.
     */
    class CppSpecTransformer : public SourceSpecTransformer
    {
    public:
        CppSpecTransformer(
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
    };

}

#endif // PALLAS_CPPSPECTRANSFORMER_H