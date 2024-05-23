#ifndef PALLAS_SOURCECOMPILERDRIVER_H
#define PALLAS_SOURCECOMPILERDRIVER_H

#include <string>
#include <memory>

#include <llvm/IR/Module.h>


namespace sourcespec2ir
{

    /**
     * Abstract class is used to invoke the compiler of the source-language in order to 
     * compile the expressions of the source-lnaguage into LLVM-IR.
     */
    class SourceCompilerDriver
    {
    public:
        /**
         * Takes a string that contains a compilation unit of the source-language and returns a LLVM-Module that contains the 
         * compiled LLVM-representation of this function.
         * Takes a string-representation of the source-language comilation unit as a parameter. 
         * 
         * The compilation is expected to generate debug information whch is needed for further processing.  
         */
        virtual std::unique_ptr<llvm::Module> compileSpecification(const std::string &cu) = 0;

    };

}

#endif // PALLAS_SOURCECOMPILERDRIVER_H