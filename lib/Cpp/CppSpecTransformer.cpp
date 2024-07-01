#include "CppSpecTransformer.h"
#include "CppWrapperGenerator.h"

namespace sourcespec2ir
{

    CppSpecTransformer::CppSpecTransformer(
        std::unique_ptr<SourceCompilerDriver> compilerDriver,
        std::unique_ptr<SpecUnifier> specUnifier,
        std::string srcInPath, std::string outPath,
        const utils::OutputConfig &outputConfig)
        : SourceSpecTransformer(std::move(compilerDriver),
                                std::move(specUnifier), 
                                srcInPath, 
                                outPath, 
                                outputConfig)
    {
    }

    std::unique_ptr<SourceWrapperGenerator> CppSpecTransformer::createWrapperGenerator()
    {
        return std::make_unique<CppWrapperGenerator>();
    }
}