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

        // Check if the name starts with the the prefix of the spec module
        if (!llvm::StringRef(fName).startswith(SWIFT_SPEC_MODULE_PREFIX)) 
            return std::nullopt;

        auto nameWithoutPrefix = fName.substr(SWIFT_SPEC_MODULE_PREFIX.size());

        // Check exact matches
        auto exactMatch = EXACT_MATCH_NAME_MAP.find(nameWithoutPrefix);
        if (exactMatch != EXACT_MATCH_NAME_MAP.end())
            return std::make_optional<SpecLibElement>(exactMatch->getValue());

        // Check prefix-matches
        for (auto key : PREFIX_MATCH_NAME_MAP.keys())
        {
            if (llvm::StringRef(nameWithoutPrefix).startswith(key))
            {
                auto value = PREFIX_MATCH_NAME_MAP.lookup(key);
                return std::make_optional<SpecLibElement>(value);
            }
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
        // TODO: Implment properly
        return nullptr;
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
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformResult(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
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
        throw utils::PallasException("Unsupported specification construct");
    }

    llvm::CallInst *SwiftSpecUnifier::transformPerm(llvm::CallInst &oldCall)
    {
        throw utils::PallasException("Unsupported specification construct");
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