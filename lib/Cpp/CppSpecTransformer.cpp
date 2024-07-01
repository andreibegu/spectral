#include "CppCompilerDriver.h"
#include "CppSpecUnifier.h"
#include "CppSpecTransformer.h"
#include "CppWrapperGenerator.h"

namespace sourcespec2ir
{

    CppSpecTransformer::CppSpecTransformer(
        std::string specLibPath,
        std::string srcInPath, std::string outPath,
        const utils::OutputConfig &outputConfig)
        : SourceSpecTransformer(
            std::make_unique<CppCompilerDriver>(specLibPath),
            std::make_unique<CppSpecUnifier>(outputConfig.printUnificationInfo),
            srcInPath, outPath, outputConfig)
    {
    }

    std::unique_ptr<SourceWrapperGenerator> CppSpecTransformer::createWrapperGenerator()
    {
        return std::make_unique<CppWrapperGenerator>();
    }
}