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

} // namespace sourcespec2ir