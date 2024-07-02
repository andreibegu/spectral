#include "CppSpecUnifier.h"
#include "PallasException.h"

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
        auto fName = f.getName();
        auto nameDemangle = llvm::demangle(fName.str());
        llvm::outs() << "Demangle " << fName << " --> " << nameDemangle << "\n";


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