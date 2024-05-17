#ifndef PALLAS_IRVARIABLECOLLECTOR_H
#define PALLAS_IRVARIABLECOLLECTOR_H

#include <llvm/IR/Function.h>
#include <string>


namespace sourcespec2ir
{
    /**
     * Class that analyzes a function of LLVM-IR and returns the names and types of all 
     * source-language variables that are part of the function.
     * 
     * It utilizes the debug-information of the LLVM-IR to retrieve the source-level variables. 
     * Currently, the LLVm debug-intrinsics are used to retrieve the variables. 
     * 
     * TODO: Since these will be deprecated with LLVM-19 (https://llvm.org/docs/RemoveDIsDebugInfo.html) this will have to 
     * be migrated once we switch to LLVM-19. 
     */
    class IRVariableCollector
    {
    public:
        IRVariableCollector(std::string srcFunctionName, std::string irFilePath);
    private:
        void getVariablesForFunction(const llvm::Function &fun);
    };

}

#endif // PALLAS_IRVARIABLECOLLECTOR_H