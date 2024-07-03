#include "CppSpecUnifier.h"
#include "PallasException.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/Support/raw_ostream.h>

namespace sourcespec2ir
{

    CppSpecUnifier::CppSpecUnifier(const bool printDebugInfo)
        : SpecUnifier(printDebugInfo)
    {
    }

    std::optional<SpecUnifier::SpecLibElement> CppSpecUnifier::isSrcSpecificationFunction(const llvm::Function &f)
    {
        // Implement: Make less bad, check scope
        auto fName = f.getName().str();
        auto nameDemangle = llvm::demangle(fName);
        // llvm::outs() << "Demangle " << fName << " --> " << nameDemangle << "\n";

        // Check if the name contains the spec-namespace:
        auto namespacePos = llvm::StringRef(nameDemangle).find(CPP_SPEC_NAMESPACE + "::");
        if (namespacePos == llvm::StringRef::npos)
            return std::nullopt;
        // Remove namespace from the demangled name
        auto nameCrop = nameDemangle.substr(
            namespacePos + CPP_SPEC_NAMESPACE.length() + 2);

        // Check exact matches
        auto exactMatch = EXACT_MATCH_NAME_MAP.find(nameCrop);
        if (exactMatch != EXACT_MATCH_NAME_MAP.end())
        {
            return std::make_optional<SpecLibElement>(exactMatch->getValue());
        }

        // Check prefix-matches
        for (auto key : PREFIX_MATCH_NAME_MAP.keys())
        {
            if (llvm::StringRef(nameCrop).startswith(key))
            {
                auto value = PREFIX_MATCH_NAME_MAP.lookup(key);
                return std::make_optional<SpecLibElement>(value);
            }
        }

        return std::nullopt;
    }

    llvm::Type *CppSpecUnifier::getContentTypeOfADTTy(const llvm::Type &t)
    {
        throw std::runtime_error("Not implemented");
    }

    bool CppSpecUnifier::isSrcSeqType(const llvm::StructType &t)
    {
        return t.getName().startswith("struct." + CPP_SPEC_SEQ_TYPE_BASE);
    }

    llvm::Type *CppSpecUnifier::getSrcLibFracType()
    {
        return llvm::StructType::getTypeByName(*irContext, CPP_SPEC_FRAC_TYPE);
    }

    llvm::CallInst *CppSpecUnifier::transformImply(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecImply(), newArgs);
    }

    llvm::CallInst *CppSpecUnifier::transformOld(llvm::CallInst &oldCall)
    {
        auto type = oldCall.getCalledFunction()->getReturnType();
        llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
        return llvm::CallInst::Create(getUSpecOld(type), newArgs);
    }

    llvm::CallInst *CppSpecUnifier::transformResult(llvm::CallInst &oldCall)
    {
        auto type = oldCall.getCalledFunction()->getReturnType();
        llvm::SmallVector<llvm::Value *, 1> newArgs;
        return llvm::CallInst::Create(getUSpecResult(type), newArgs);
    }

    llvm::CallInst *CppSpecUnifier::transformBoundVar(llvm::CallInst &oldCall)
    {
        auto type = oldCall.getCalledFunction()->getReturnType();
        llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
        return llvm::CallInst::Create(getUSpecBoundVar(type), newArgs);
    }

    llvm::CallInst *CppSpecUnifier::transformExists(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecExists(), newArgs);
    }

    llvm::CallInst *CppSpecUnifier::transformForall(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecForall(), newArgs);
    }

    llvm::CallInst *CppSpecUnifier::transformForallSep(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecForallSep(), newArgs);
    }

    llvm::CallInst *CppSpecUnifier::transformFracOf(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 4> newArgs = {
            oldCall.getArgOperand(0), oldCall.getArgOperand(1),
            oldCall.getArgOperand(2)};
        auto newCall = llvm::CallInst::Create(getUSpecFracOf(), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecFracType()));
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        newCall->addParamAttr(2, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *CppSpecUnifier::transformPerm(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        auto newCall = llvm::CallInst::Create(getUSpecPerm(), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        newCall->addParamAttr(1, llvm::Attribute::getWithByValType(
            *irContext, getUSpecFracType()));
        return newCall;
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