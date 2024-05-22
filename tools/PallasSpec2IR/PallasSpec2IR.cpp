#include <iostream>
#include <memory>

#include "antlr4-runtime.h"
#include "transform/SourceSpecTransformer.h"
#include "transform/DummySourceSpecContextProvider.h"
#include "transform/SourceSpecContextProvider.h"

int main(int argc, char **argv)
{
    
    auto specCtxProvider = std::make_unique<sourcespec2ir::DummySourceSpecContextProvider>("bar");
    sourcespec2ir::SourceSpecTransformer transformer(std::move(specCtxProvider), argv[2], argv[1]);

    // transformer.printSpecifications(argv[1], argv[2]);
    transformer.transformModule();

    return 0;
}