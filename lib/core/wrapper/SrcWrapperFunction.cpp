#include "PallasException.h"
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
        givenArgs(givenArgs.begin(), givenArgs.end()),
        yieldsArgs(yieldsArgs.begin(), yieldsArgs.end()),
        insertLoc(insertLoc)
  {
    // No renamig of args, map DIVars directly to their name.
    for (auto *a : argList)
    {
      pVarsToArgNames.insert({a, a->getName().str()});
    }
  }

  SrcWrapperFunction::SrcWrapperFunction(
      std::string name, std::string stringRepr,
      llvm::ArrayRef<llvm::DILocalVariable *> argList,
      llvm::ArrayRef<std::string> argNames,
      llvm::ArrayRef<std::string> givenArgs,
      llvm::ArrayRef<std::string> yieldsArgs,
      size_t insertLoc)
      : name(name),
        stringRepr(stringRepr),
        givenArgs(givenArgs),
        yieldsArgs(yieldsArgs)
  {
    if (argList.size() != argNames.size()) {
      throw utils::PallasException(
        "Attempted to construct a SrcWrapperFunction but the number of "
        "variables and arg-names does not match!");
    }

    for (auto [v, n] : llvm::zip_equal(argList, argNames)) {
      pVarsToArgNames.insert({v, n});
    }
  }

}