#include "SwiftSpecUnifier.h"
#include "IrLookup.h"
#include "PallasException.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/Support/raw_ostream.h>

namespace sourcespec2ir
{

    SwiftSpecUnifier::SwiftSpecUnifier(const bool printDebugInfo)
        : SpecUnifier(printDebugInfo)
    {
    }

    std::optional<SpecUnifier::SpecLibElement>
    SwiftSpecUnifier::isSrcSpecificationFunction(const llvm::Function &f)
    {
        auto fName = f.getName().str();

        if (llvm::StringRef(fName).startswith(SWIFT_SPEC_MODULE_PREFIX))
        {
            // The name starts with the prefix of the spec-module
            auto nameWithoutPrefix = fName.substr(SWIFT_SPEC_MODULE_PREFIX.size());
            // Check name-matches
            auto match = EXACT_MATCH_NAME_MAP.find(nameWithoutPrefix);
            if (match != EXACT_MATCH_NAME_MAP.end())
                return std::make_optional<SpecLibElement>(match->getValue());
        }

        // Check if it is a generated spec-function inside of the wrapper
        if (auto sProg = f.getSubprogram())
        {
            if (sProg->getScope() == nullptr ||
                !sProg->getScope()->getName().startswith(SWIFT_WRAPPER_PREFIX))
                return std::nullopt;

            auto match = GENERATED_NAME_MAP.find(sProg->getName());
            if (match != GENERATED_NAME_MAP.end())
                return std::make_optional<SpecLibElement>(match->getValue());
        }

        return std::nullopt;
    }

    llvm::Type *SwiftSpecUnifier::getContentTypeOfADTTy(const llvm::Type &t)
    {
        throw utils::PallasException("Not implemented yet");
    }

    bool SwiftSpecUnifier::isSrcSeqType(const llvm::StructType &t)
    {
        // TODO: Implment properly
        return false;
    }

    llvm::Type *SwiftSpecUnifier::getSrcLibFracType()
    {
        return llvm::StructType::getTypeByName(*irContext, SWIFT_SPEC_FRAC_TYPE);
    }

    llvm::CallInst *SwiftSpecUnifier::transformImply(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecImply(), newArgs);
    }

    llvm::CallInst *SwiftSpecUnifier::transformSepConj(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecSepConj(), newArgs);
    }

    llvm::CallInst *SwiftSpecUnifier::transformOld(llvm::CallInst &oldCall)
    {
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            // Struct-type
            auto newCall = llvm::CallInst::Create(
                getUSpecOld(type), {oldCall.getArgOperand(0), oldCall.getArgOperand(1)});
            addSret(newCall, 0, type);
            addByVal(newCall, 1, type);
            return newCall;
        }
        else
        {
            return llvm::CallInst::Create(
                getUSpecOld(type), {oldCall.getArgOperand(0)});
        }
    }

    llvm::CallInst *SwiftSpecUnifier::transformResult(llvm::CallInst &oldCall)
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

    llvm::CallInst *SwiftSpecUnifier::transformBoundVar(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformExists(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformForall(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformForallSep(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformFracOf(llvm::CallInst &oldCall)
    {
        // throw utils::PallasException("Unsupported specification construct");
        llvm::SmallVector<llvm::Value *, 4> newArgs = {
            oldCall.getArgOperand(0), oldCall.getArgOperand(1),
            oldCall.getArgOperand(2)};
        auto newCall = llvm::CallInst::Create(getUSpecFracOf(), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecFracType()));
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        newCall->addParamAttr(2, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *SwiftSpecUnifier::transformPerm(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
        auto newCall = llvm::CallInst::Create(getUSpecPerm(), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        // TODO: Remove the byval attribute and update the documentation of the spec-lib. 
        newCall->addParamAttr(1, llvm::Attribute::getWithByValType(
                                     *irContext, getUSpecFracType()));
        return newCall;
    }

    llvm::CallInst *SwiftSpecUnifier::transformSeqNew(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformSeqSize(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformSeqGet(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformSeqPrepend(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformSeqEquals(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
    }

} // namespace sourcespec2ir