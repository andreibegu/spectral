#ifndef PALLAS_CCOMPILERDRIVER_H
#define PALLAS_CCOMPILERDRIVER_H

#include "transform/SourceCompilerDriver.h"


namespace sourcespec2ir
{

    /**
     * Driver class that invokes clang to compile specifications to LLVM-IR.
     */
    class CCompilerDriver : public SourceCompilerDriver
    {
    public:
        CCompilerDriver(std::string specLibPath);

        std::unique_ptr<llvm::Module> compileSource(const std::string &srcFilePath, llvm::LLVMContext &context);

        std::unique_ptr<llvm::Module> compileSpecification(const std::string &cu, llvm::LLVMContext &context);

    private:
        std::string specLibPath;
    };

}

#endif // PALLAS_CCOMPILERDRIVER_H