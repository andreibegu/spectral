#ifndef PALLAS_CPPCOMPILERDRIVER_H
#define PALLAS_CPPCOMPILERDRIVER_H

#include "SourceCompilerDriver.h"


namespace sourcespec2ir
{

    /**
     * Driver class that invokes clang++ to compile specifications to LLVM-IR.
     */
    class CppCompilerDriver : public SourceCompilerDriver
    {
    public:
        CppCompilerDriver(std::string specLibPath);

        std::unique_ptr<llvm::Module> compileSource(
            const std::string &srcFilePath, llvm::LLVMContext &context);

        std::unique_ptr<llvm::Module> compileSpecification(
            const std::string &cu, llvm::LLVMContext &context);

    private:
        std::string specLibPath;
    };

}

#endif // PALLAS_CPPCOMPILERDRIVER_H