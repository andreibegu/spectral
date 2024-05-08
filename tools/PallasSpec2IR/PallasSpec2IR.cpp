#include <iostream>

#include "antlr4-runtime.h"
#include "transform/SourceSpecTransformer.h"

int main(int argc, char **argv)
{

    sourcespec2ir::SourceSpecTransformer transformer;
    transformer.printSpecifications(argv[1]);

    return 0;
}