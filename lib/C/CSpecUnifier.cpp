#include "CSpecUnifier.h"
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

} // namespace sourcespec2ir