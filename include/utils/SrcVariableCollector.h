#ifndef PALLAS_SRCVARIABLECOLLECTOR_H
#define PALLAS_SRCVARIABLECOLLECTOR_H

#include <string>
#include <memory>

#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

namespace sourcespec2ir
{

    /**
     * Helper functions that extract the names and types of source-language variables from an IR function.
     *
     * Utilizes the debug-information of the LLVM-IR to retrieve the source-level variables.
     * Currently, the LLVm debug-intrinsics are used to retrieve the variables.
     *
     * TODO: Since these will be deprecated with LLVM-19 (https://llvm.org/docs/RemoveDIsDebugInfo.html) this will have to
     * be migrated once we switch to LLVM-19.
     * 
     * TODO: Rename to something better!
     */

    /**
     * Collects the source-level names and types for the varibles in the given function 'fun'.
     * Parameter 'argsOnly' determines if only function arguments are considered.
     * The detection of function-arguments assumes that the mem2reg pass has been run on the IR of the representation.
     * Currently, global variables are not returned when argsOnly is set to true.
     */
    std::unique_ptr<llvm::StringMap<std::string>> getSrcVariablesForFunction(const llvm::Function &fun, const bool argsOnly);

    llvm::Function *getFunctionBySrcName(llvm::Module &module, const std::string &src_name);

    /**
     * Prints all the source-vars and their types. For testing purposes
     */
    void printAllSrcVars(std::string srcFunctionName, std::string irFilePath);

}

#endif // PALLAS_SRCVARIABLECOLLECTOR_H