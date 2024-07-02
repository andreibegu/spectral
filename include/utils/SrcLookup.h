#ifndef PALLAS_SRCLOOKUP_H
#define PALLAS_SRCLOOKUP_H

#include <string>

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

namespace sourcespec2ir
{
    namespace utils
    {
        /**
         * Helper functions that lookup information regarding the source-level 
         * representation from the LLVM-IR.
         */

        /*
         * Returns a pointer to the IR-function that corresponds to the 
         * source-level function with the given name. 
         */
        llvm::Function *getFunctionBySrcName(
            llvm::Module &module, const std::string &src_name);

        /**
         * Returns a string that represents the name of the given function in 
         * the source-file. If the name cannot be determined, an empty string is 
         * returned. 
         */
        std::string getSrcNameOfFunction(const llvm::Function &f);

        /**
         * Returns the source name of the given DILocalVariable 
         */
        std::string getSrcVarName(const llvm::DILocalVariable &var);

    }
}

#endif // PALLAS_SRCLOOKUP_H