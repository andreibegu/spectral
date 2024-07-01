#include "CppSpecUnifier.h"
#include "PallasException.h"

namespace sourcespec2ir
{

CppSpecUnifier::CppSpecUnifier(const bool printDebugInfo)
    : SpecUnifier(printDebugInfo)
{}

std::optional<SpecUnifier::SpecLibElement> CppSpecUnifier::isSrcSpecificationFunction(const llvm::Function &f)
{
    throw std::runtime_error("Not implemented");
}

llvm::Type *CppSpecUnifier::getContentTypeOfADTTy(const llvm::Type &t)
{
    throw std::runtime_error("Not implemented");
}

bool CppSpecUnifier::isSrcSeqType(const llvm::StructType &t)
{
    throw std::runtime_error("Not implemented");
}

llvm::Type *CppSpecUnifier::getSrcLibFracType()
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformImply(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformOld(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformResult(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformBoundVar(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformExists(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformForall(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformForallSep(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformFracOf(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformPerm(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformSeqNew(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformSeqSize(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformSeqGet(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

llvm::CallInst *CppSpecUnifier::transformSeqAppend(llvm::CallInst &oldCall)
{
    throw std::runtime_error("Not implemented");
}

} // namespace sourcespec2ir