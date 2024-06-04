#include <memory>

#include "transform/SourceSpecTransformer.h"
#include "transform/DummySourceSpecContextProvider.h"
#include "transform/SourceSpecContextProvider.h"
#include "transform/SpecUnifier.h"
#include "transform/CCompilerDriver.h"

int main(int argc, char **argv)
{
    
    auto specCtxProvider = std::make_unique<sourcespec2ir::DummySourceSpecContextProvider>("bar");
    auto compilerDriver = std::make_unique<sourcespec2ir::CCompilerDriver>();
    auto specUnifier = std::make_unique<sourcespec2ir::SpecUnifier>();
    sourcespec2ir::SourceSpecTransformer transformer(std::move(specCtxProvider), std::move(compilerDriver), 
        std::move(specUnifier), argv[1]);

    transformer.transformModule();

    return 0;
}