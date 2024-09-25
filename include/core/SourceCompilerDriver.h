#ifndef PALLAS_SOURCECOMPILERDRIVER_H
#define PALLAS_SOURCECOMPILERDRIVER_H

#include <string>
#include <memory>

#include <llvm/IR/Module.h>


namespace sourcespec2ir
{

    /**
     * Abstract class is used to invoke the compiler of the source-language to 
     * compile expressions of the source-language into LLVM-IR.
     */
    class SourceCompilerDriver
    {
    public:
        /** 
         * Creates a new SourceCompilerDriver that uses the given 
         * working-directory.
         */
        SourceCompilerDriver(const std::string &workingDir);

        virtual ~SourceCompilerDriver() = default;

        /**
         * Compiles the provided source-file into an IR-representation. 
         * Returns a pointer to the IR representation and transfers ownership 
         * to the caller. 
         */
        virtual std::unique_ptr<llvm::Module> compileSource(
            const std::string &srcFilePath, llvm::LLVMContext &context) = 0;

    protected:
        const std::string workingDir;
    };

}

#endif // PALLAS_SOURCECOMPILERDRIVER_H