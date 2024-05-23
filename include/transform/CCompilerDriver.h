#ifndef PALLAS_CCOMPILERDRIVER_H
#define PALLAS_CCOMPILERDRIVER_H

#include "SourceCompilerDriver.h"


namespace sourcespec2ir
{

    /**
     * Driver class that invokes clang to compile specifications to LLVM-IR.
     */
    class CCompilerDriver : public SourceCompilerDriver
    {
    public:

        std::unique_ptr<llvm::Module> compileSpecification(const std::string &cu);

    };

}

#endif // PALLAS_CCOMPILERDRIVER_H