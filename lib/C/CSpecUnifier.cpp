#include "CSpecUnifier.h"
#include "IrLookup.h"
#include "PallasException.h"

namespace sourcespec2ir
{

    CSpecUnifier::CSpecUnifier(const bool printDebugInfo)
        : SpecUnifier(printDebugInfo)
    {
    }

    std::optional<SpecUnifier::SpecLibElement> CSpecUnifier::isSrcSpecificationFunction(const llvm::Function &f)
    {
        // TODO: Implement this better (i.e. check for scope or sth like that)

        auto fName = f.getName();
        // Check exact matches
        auto exactMatch = EXACT_MATCH_NAME_MAP.find(fName);
        if (exactMatch != EXACT_MATCH_NAME_MAP.end())
        {
            return std::make_optional<SpecLibElement>(exactMatch->getValue());
        }

        // Check prefix-matches
        for (auto key : PREFIX_MATCH_NAME_MAP.keys())
        {
            if (fName.startswith(key))
            {
                auto value = PREFIX_MATCH_NAME_MAP.lookup(key);
                return std::make_optional<SpecLibElement>(value);
            }
        }

        return std::nullopt;
    }

    llvm::Type *CSpecUnifier::getContentTypeOfADTTy(const llvm::Type &t)
    {
        if (auto *adtType = llvm::dyn_cast<llvm::StructType>(&t))
        {
            return adtType->getElementType(0);
        }
        throw utils::PallasException("Failed to extract the content-type of the sequence-type.");
    }

    bool CSpecUnifier::isSrcSeqType(const llvm::StructType &t)
    {
        return t.getName().startswith("struct." + C_SPEC_SEQ_TYPE_BASE);
    }

    llvm::Type *CSpecUnifier::getSrcLibFracType()
    {
        return llvm::StructType::getTypeByName(*irContext, C_SPEC_FRAC_TYPE);
    }

    llvm::CallInst *CSpecUnifier::transformImply(llvm::CallInst &oldCall)
    {
        // Imply
        llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        // If we had to do some argument translation, this would happen here
        return llvm::CallInst::Create(getUSpecImply(), newArgs);
    }

    llvm::CallInst *CSpecUnifier::transformOld(llvm::CallInst &oldCall)
    {
        // \old
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            // Struct-type
            auto newCall = llvm::CallInst::Create(
                getUSpecOld(type), {oldCall.getArgOperand(0), oldCall.getArgOperand(1)});
            addSret(newCall, 0, type);
            return newCall;
        }
        else
        {
            return llvm::CallInst::Create(
                getUSpecOld(type), {oldCall.getArgOperand(0)});
        }
    }

    llvm::CallInst *CSpecUnifier::transformResult(llvm::CallInst &oldCall)
    {
        // \result
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            auto newCall = llvm::CallInst::Create(
                getUSpecResult(type), {oldCall.getArgOperand(0)});
            addSret(newCall, 0, type);
            return newCall;
        }
        else
        {
            llvm::SmallVector<llvm::Value *, 1> newArgs;
            return llvm::CallInst::Create(getUSpecResult(type), newArgs);
        }
    }

    llvm::CallInst *CSpecUnifier::transformBoundVar(llvm::CallInst &oldCall)
    {
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0), 
                oldCall.getArgOperand(1)};
            auto newCall = llvm::CallInst::Create(getUSpecBoundVar(type), newArgs);
            addSret(newCall, 0, type);
            return newCall;
        }
        else
        {
            llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
            return llvm::CallInst::Create(getUSpecBoundVar(type), newArgs);
        }
    }

    llvm::CallInst *CSpecUnifier::transformExists(llvm::CallInst &oldCall)
    {
        // exists
        llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecExists(), newArgs);
    }

    llvm::CallInst *CSpecUnifier::transformForall(llvm::CallInst &oldCall)
    {
        // forall
        llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecForall(), newArgs);
    }

    llvm::CallInst *CSpecUnifier::transformForallSep(llvm::CallInst &oldCall)
    {
        // forall*
        llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecForallSep(), newArgs);
    }

    llvm::CallInst *CSpecUnifier::transformFracOf(llvm::CallInst &oldCall)
    {
        // fracOf
        llvm::SmallVector<llvm::Value *, 4> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1),
                                                       oldCall.getArgOperand(2)};
        auto newCall = llvm::CallInst::Create(getUSpecFracOf(), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecFracType()));
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        newCall->addParamAttr(2, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *CSpecUnifier::transformPerm(llvm::CallInst &oldCall)
    {
        // Perm
        llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        auto newCall = llvm::CallInst::Create(getUSpecPerm(), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        newCall->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, getUSpecFracType()));
        return newCall;
    }

    llvm::CallInst *CSpecUnifier::transformSeqNew(llvm::CallInst &oldCall)
    {
        // New sequence
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
        llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
        auto newCall = llvm::CallInst::Create(getUSpecSeqNew(cType), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(cType)));
        return newCall;
    }

    llvm::CallInst *CSpecUnifier::transformSeqSize(llvm::CallInst &oldCall)
    {
        // Size of sequences
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
        llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
        auto newCall = llvm::CallInst::Create(getUSpecSeqSize(cType), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        newCall->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(cType)));
        return newCall;
    }

    llvm::CallInst *CSpecUnifier::transformSeqGet(llvm::CallInst &oldCall)
    {
        // Check the signature of the old call to determine if a
        // struct-type is returned.
        bool containsStruct = oldCall.getCalledFunction()->getReturnType() == llvm::Type::getVoidTy(*irContext);

        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(containsStruct ? 1 : 0));
        llvm::SmallVector<llvm::Value *, 4> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1)};
        if (containsStruct)
            newArgs.push_back(oldCall.getArgOperand(2));
        auto newCall = llvm::CallInst::Create(getUSpecSeqGet(cType), newArgs);
        unsigned int argIdx = 0;
        if (containsStruct)
            addSret(newCall, argIdx++, cType);
        newCall->addParamAttr(argIdx, llvm::Attribute::NoUndef);
        newCall->addParamAttr(argIdx++, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(cType)));
        newCall->addParamAttr(argIdx, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *CSpecUnifier::transformSeqAppend(llvm::CallInst &oldCall)
    {
        // Append on sequences
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