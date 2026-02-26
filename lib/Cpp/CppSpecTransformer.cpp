#include "CppCompilerDriver.h"
#include "CppSpecUnifier.h"
#include "CppSpecTransformer.h"
#include "CppWrapperGenerator.h"

namespace sourcespec2ir
{

    CppSpecTransformer::CppSpecTransformer(
        std::string specLibPath,
        std::string srcInPath, std::string outPath,
        llvm::SmallVector<std::string> extContractPaths,
        const utils::OutputConfig &outputConfig,
        const std::string &workingDir,
        bool runMem2Reg)
        : SourceSpecTransformer(
              std::make_unique<CppCompilerDriver>(specLibPath, workingDir),
              std::make_unique<CppSpecUnifier>(outputConfig.printUnificationInfo),
              srcInPath, outPath, extContractPaths, outputConfig, workingDir,
              runMem2Reg)
    {
    }

    std::unique_ptr<SourceWrapperGenerator>
    CppSpecTransformer::createWrapperGenerator(
        const llvm::SmallVector<SpecDeclaration> &specDeclarations,
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers)
    {
        return std::make_unique<CppWrapperGenerator>(
            getSrcInPath(), specDeclarations, wrapperMarkers,
            createSyntaxResolver());
    }
}