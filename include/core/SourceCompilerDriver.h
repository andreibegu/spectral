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
         * Returns a pointer to the IR representation and transfers ownership to the caller. 
         */
        virtual std::unique_ptr<llvm::Module> compileSource(const std::string &srcFilePath, llvm::LLVMContext &context) = 0;


        /**
         * Takes a string that contains a compilation unit of the source-language and returns a LLVM-Module that contains the 
         * compiled LLVM-representation of this function.
         * Takes a string-representation of the source-language comilation unit as a parameter. 
         * 
         * The compilation is expected to generate debug information whch is needed for further processing.  
         */
        virtual std::unique_ptr<llvm::Module> compileSpecification(const std::string &cu, llvm::LLVMContext &context) = 0;

    };

}

#endif // PALLAS_SOURCECOMPILERDRIVER_H