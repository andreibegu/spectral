#include "wrapper/SrcWrapperFunction.h"

namespace sourcespec2ir
{

    SrcWrapperFunction::SrcWrapperFunction(std::string name, std::string stringRepr,
                                           const llvm::ArrayRef<llvm::DILocalVariable *> &argList)
        : name(name),
          stringRepr(stringRepr),
          argList(argList)
    {
    }
}