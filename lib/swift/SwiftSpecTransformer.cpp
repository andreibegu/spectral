#include "SwiftCompilerDriver.h"
#include "SwiftSpecUnifier.h"
#include "SwiftSpecTransformer.h"
#include "SwiftWrapperGenerator.h"

namespace sourcespec2ir
{

    SwiftSpecTransformer::SwiftSpecTransformer(
        std::string specLibPath,
        std::string srcInPath, std::string outPath,
        llvm::SmallVector<std::string> extContractPaths,
        const utils::OutputConfig &outputConfig,
        const std::string &workingDir, 
        bool runMem2Reg)
        : SourceSpecTransformer(
              std::make_unique<SwiftCompilerDriver>(specLibPath, workingDir),
              std::make_unique<SwiftSpecUnifier>(outputConfig.printUnificationInfo),
              srcInPath, outPath, extContractPaths, outputConfig, workingDir, 
              runMem2Reg)
    {
    }

    std::unique_ptr<SourceWrapperGenerator> 
    SwiftSpecTransformer::createWrapperGenerator(
        const llvm::SmallVector<SpecDeclaration> &specDeclarations)
    {
        return std::make_unique<SwiftWrapperGenerator>(specDeclarations);
    }
}