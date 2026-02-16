#include "wrapper/SrcWrapperFunction.h"

namespace sourcespec2ir
{

  SrcWrapperFunction::SrcWrapperFunction(
      std::string name, std::string stringRepr,
      llvm::ArrayRef<llvm::DILocalVariable *> argList,
      llvm::ArrayRef<std::string> givenArgs,
      llvm::ArrayRef<std::string> yieldsArgs,
      size_t insertLoc)
      : name(name),
        stringRepr(stringRepr),
        argList(argList.begin(), argList.end()),
        givenArgs(givenArgs.begin(), givenArgs.end()),
        yieldsArgs(yieldsArgs.begin(), yieldsArgs.end()),
        insertLoc(insertLoc)
  {
  }
}