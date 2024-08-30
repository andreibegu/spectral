#ifndef PALLAS_SWIFTCOMPILERDRIVER_H
#define PALLAS_SWIFTCOMPILERDRIVER_H

#include "SourceCompilerDriver.h"

namespace sourcespec2ir
{

    /**
     * Driver class that invokes swiftc to compile specifications to LLVM-IR.
     */
    class SwiftCompilerDriver : public SourceCompilerDriver
    {
    public:
        SwiftCompilerDriver(std::string specLibPath, const std::string &workingDir);

        std::unique_ptr<llvm::Module> compileSource(
            const std::string &srcFilePath, llvm::LLVMContext &context);

    private:
        std::string specLibPath;
        // Counter that is used to generate different filenames every time the 
        // compiler-driver is called. 
        std::size_t callCounter = 0;
    };

}

#endif // PALLAS_SWIFTCOMPILERDRIVER_H