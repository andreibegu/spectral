#ifndef PALLAS_IRVARIABLECOLLECTOR_H
#define PALLAS_IRVARIABLECOLLECTOR_H

#include <llvm/IR/Function.h>
#include <string>


namespace sourcespec2ir
{
    /**
     * Class that analyses the LLVM-IR and returns the names and types of all variables 
     * in this function.   
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