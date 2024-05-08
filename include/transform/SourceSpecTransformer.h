#ifndef PALLAS_SOURCESPECTRANSFORMER_H
#define PALLAS_SOURCESPECTRANSFORMER_H

#include "SpecParserBaseVisitor.h"

namespace sourcespec2ir
{

    class SourceSpecTransformer : SpecParserBaseVisitor
    {
    public:
        void printSpecifications(char *inputPath);
        void test(char *inputPath);
    };

}

#endif // PALLAS_SOURCESPECTRANSFORMER_H