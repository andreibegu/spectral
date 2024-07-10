#include "wrapper/SrcWrapperFunction.h"

namespace sourcespec2ir
{

  SrcWrapperFunction::SrcWrapperFunction(std::string name, std::string stringRepr,
                                         llvm::ArrayRef<llvm::DILocalVariable *> argList,
                                         size_t insertLoc)
      : name(name),
        stringRepr(stringRepr),
        argList(argList.begin(), argList.end()),
        insertLoc(insertLoc)
  {
  }
}