#include "IrLookup.h"

#include <llvm/IR/Attributes.h>

namespace sourcespec2ir
{
    namespace utils
    {
        
        llvm::Type *getReturnType(const llvm::Function &f)
        {
            if (f.hasParamAttribute(0, llvm::Attribute::StructRet))
                return f.getParamStructRetType(0);
            if (f.hasParamAttribute(1, llvm::Attribute::StructRet))
                return f.getParamStructRetType(1);
            return f.getReturnType();
        }

    }

}