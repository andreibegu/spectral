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

    std::optional<SpecUnifier::SpecLibElement>
    CppSpecUnifier::isSrcSpecificationFunction(const llvm::Function &f)
    {
        // Implement: Make less bad, check scope
        auto fName = f.getName();
        // auto nameDemangle = llvm::demangle(fName);

        // Check that the name blongs to the spec-lib namespace: 
        if (!fName.starts_with(CPP_SPEC_NAMESPACE)) {
            return std::nullopt;
        } 
        // Remove the namespace: 
        auto nameCrop = fName.drop_front(CPP_SPEC_NAMESPACE.size());

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

} // namespace sourcespec2ir