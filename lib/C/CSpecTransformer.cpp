#include "CSpecTransformer.h"
#include "CWrapperGenerator.h"

namespace sourcespec2ir
{

    CSpecTransformer::CSpecTransformer(
        std::unique_ptr<SourceCompilerDriver> compilerDriver,
        std::unique_ptr<SpecUnifier> specUnifier,
        std::string srcInPath, std::string outPath,
        const utils::OutputConfig &outputConfig,
        const std::string &workingDir, 
        bool runMem2Reg)
        : SourceSpecTransformer(std::move(compilerDriver),
                                std::move(specUnifier),
                                srcInPath,
                                outPath,
                                outputConfig,
                                workingDir, 
                                runMem2Reg)
    {
    }

    std::unique_ptr<SourceWrapperGenerator> 
    CSpecTransformer::createWrapperGenerator(
        const llvm::SmallVector<SpecDeclaration> &specDeclarations)
    {
        return std::make_unique<CWrapperGenerator>(specDeclarations);
    }
}