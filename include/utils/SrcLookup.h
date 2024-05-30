#ifndef PALLAS_SRCLOOKUP_H
#define PALLAS_SRCLOOKUP_H

#include <string>
#include <memory>

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

namespace sourcespec2ir
{
    namespace utils
    {
        /**
         * Helper functions that lookup information regarding the source-level representation from the LLVM-IR.
         */

        /*
         * Returns a pointer to the IR-function that corresponds to the source-level function with the given name. 
         */
        llvm::Function *getFunctionBySrcName(llvm::Module &module, const std::string &src_name);

        /**
         * Prints all the source-vars and their types. For testing purposes
         */
        void printAllSrcVars(const llvm::Module &module);

        /**
         * Returns the source name of the given DILocalVariable 
         */
        std::string getSrcVarName(const llvm::DILocalVariable &var);

        /**
         * Returns the type of the source variable that is represented by the given DILocalVariable 
         */
        std::string getSrcVarType(const llvm::DILocalVariable &var);

    }
}

#endif // PALLAS_SRCLOOKUP_H