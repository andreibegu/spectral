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
         * Compiles the provided source-file into an IR-representation. 
         * Returns a pointer to the IR representation and transfers ownership 
         * to the caller. 
         */
        virtual std::unique_ptr<llvm::Module> compileSource(
            const std::string &srcFilePath, llvm::LLVMContext &context) = 0;

    };

}

#endif // PALLAS_SOURCECOMPILERDRIVER_H