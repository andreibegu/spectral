#include "wrapper/IRWrapperFunction.h"

namespace sourcespec2ir
{

    IRWrapperFunction::IRWrapperFunction(llvm::Function *function, std::unique_ptr<llvm::SmallVector<llvm::DILocalVariable*>> argListDescr)
        : function(function), argListDescr(std::move(argListDescr))
    {
    }
}