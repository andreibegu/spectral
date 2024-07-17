#include "CSpecTransformer.h"
#include "CWrapperGenerator.h"

namespace sourcespec2ir
{

    CSpecTransformer::CSpecTransformer(
        std::unique_ptr<SourceCompilerDriver> compilerDriver,
        std::unique_ptr<SpecUnifier> specUnifier,
        std::string srcInPath, std::string outPath,
        const utils::OutputConfig &outputConfig,
        const std::string &workingDir)
        : SourceSpecTransformer(std::move(compilerDriver),
                                std::move(specUnifier),
                                srcInPath,
                                outPath,
                                outputConfig,
                                workingDir)
    {
    }

    std::unique_ptr<SourceWrapperGenerator> CSpecTransformer::createWrapperGenerator()
    {
        return std::make_unique<CWrapperGenerator>();
    }
}