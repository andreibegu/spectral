#include <memory>

#include "transform/SourceSpecTransformer.h"
#include "transform/SpecUnifier.h"
#include "transform/CCompilerDriver.h"

int main(int argc, char **argv)
{
    auto compilerDriver = std::make_unique<sourcespec2ir::CCompilerDriver>();
    auto specUnifier = std::make_unique<sourcespec2ir::SpecUnifier>();
    sourcespec2ir::SourceSpecTransformer transformer(std::move(compilerDriver), std::move(specUnifier), argv[1]);

    transformer.transformModule();

    return 0;
}