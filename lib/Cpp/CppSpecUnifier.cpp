#include "CppSpecUnifier.h"
#include "IrLookup.h"
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

    std::optional<SpecUnifier::SpecLibElement>
    CppSpecUnifier::isSrcSpecificationFunction(const llvm::Function &f)
    {
        // Implement: Make less bad, check scope
        auto fName = f.getName().str();
        auto nameDemangle = llvm::demangle(fName);
        // llvm::outs() << "Demangle " << fName << " --> " << nameDemangle << "\n";

        // Drop the return-type to avoid problems when the return-type contains
        // the namespace by finding index of last space before '('
        // and dropping everything in front.
        auto openIdx = nameDemangle.find_first_of('(');
        auto nameWithoutArgs = llvm::StringRef(nameDemangle).substr(0, openIdx);
        auto lastSpaceIdx = nameWithoutArgs.find_last_of(' ');
        if (lastSpaceIdx != llvm::StringRef::npos)
            nameDemangle = nameDemangle.substr(lastSpaceIdx);

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
        if (auto *adtType = llvm::dyn_cast<llvm::StructType>(&t))
        {
            return adtType->getElementType(0);
        }
        throw utils::PallasException("Failed to extract the content-type of the ADT.");
    }

    bool CppSpecUnifier::isSrcSeqType(const llvm::StructType &t)
    {
        return t.getName().startswith(CPP_SPEC_SEQ_TYPE_BASE);
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
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            // Struct-type
            auto newCall = llvm::CallInst::Create(
                getUSpecOld(type), {oldCall.getArgOperand(0), oldCall.getArgOperand(1)});
            newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, type));
            return newCall;
        }
        else
        {
            return llvm::CallInst::Create(
                getUSpecOld(type), {oldCall.getArgOperand(0)});
        }
    }

    llvm::CallInst *CppSpecUnifier::transformResult(llvm::CallInst &oldCall)
    {
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            auto newCall = llvm::CallInst::Create(
                getUSpecResult(type), {oldCall.getArgOperand(0)});
            newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, type));
            return newCall;
        }
        else
        {
            llvm::SmallVector<llvm::Value *, 1> newArgs;
            return llvm::CallInst::Create(getUSpecResult(type), newArgs);
        }
    }

    llvm::CallInst *CppSpecUnifier::transformBoundVar(llvm::CallInst &oldCall)
    {
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1)};
            auto newCall = llvm::CallInst::Create(getUSpecBoundVar(type), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, type));
            return newCall;
        }
        else
        {
            return llvm::CallInst::Create(
                getUSpecBoundVar(type), {oldCall.getArgOperand(0)});
        }
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
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
        llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
        auto newCall = llvm::CallInst::Create(getUSpecSeqNew(cType), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(cType)));
        return newCall;
    }

    llvm::CallInst *CppSpecUnifier::transformSeqSize(llvm::CallInst &oldCall)
    {
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
        llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
        auto newCall = llvm::CallInst::Create(getUSpecSeqSize(cType), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        newCall->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(cType)));
        return newCall;
    }

    llvm::CallInst *CppSpecUnifier::transformSeqGet(llvm::CallInst &oldCall)
    {
        // Check the signature of the old call to determine if a
        // struct-type is returned.
        bool containsStruct = oldCall.getCalledFunction()->getReturnType() == llvm::Type::getVoidTy(*irContext);
        if (containsStruct)
        {
            // Content-type is struct-type
            auto cType = getContentTypeOfADT(*oldCall.getArgOperand(1));
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1),
                oldCall.getArgOperand(2)};
            auto newCall = llvm::CallInst::Create(getUSpecSeqGet(cType), newArgs);
            addSret(newCall, 0, cType);
            newCall->addParamAttr(1, llvm::Attribute::NoUndef);
            newCall->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(cType)));
            newCall->addParamAttr(2, llvm::Attribute::NoUndef);
            return newCall;
        }
        else
        {
            // Content-type is a non-struct-type
            auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1)};
            auto newCall = llvm::CallInst::Create(getUSpecSeqGet(cType), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::NoUndef);
            newCall->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(cType)));
            newCall->addParamAttr(1, llvm::Attribute::NoUndef);
            return newCall;
        }
    }

    llvm::CallInst *CppSpecUnifier::transformSeqAppend(llvm::CallInst &oldCall)
    {
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(1));
        auto seqType = getUSpecSeqType(cType);
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1),
            oldCall.getArgOperand(2),
        };
        auto newCall = llvm::CallInst::Create(getUSpecSeqAppend(cType), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, seqType));
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        newCall->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, seqType));
        newCall->addParamAttr(2, llvm::Attribute::NoUndef);
        // If the seq contains structs, the last arg is a by-value pointer.
        if (llvm::isa<llvm::StructType>(cType))
            newCall->addParamAttr(2, llvm::Attribute::getWithByValType(*irContext, cType));
        return newCall;
    }

} // namespace sourcespec2ir