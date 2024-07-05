#include "SourceWrapperGenerator.h"
#include "PallasException.h"

#include <llvm/BinaryFormat/Dwarf.h>

namespace sourcespec2ir
{

    std::string SourceWrapperGenerator::getSrcReprOfType(const llvm::DIType *type)
    {
        // Check if type is a derived type
        if (auto *dType = llvm::dyn_cast<llvm::DIDerivedType>(type))
        {
            auto tag = dType->getTag();
            auto baseType = dType->getBaseType();

            if (tag == llvm::dwarf::DW_TAG_pointer_type)
            {
                // Handle Pointer-type
                // Recursively unwrap the pointer
                auto baseRepr = getSrcReprOfType(baseType);
                return addPointer(baseRepr);
                // return baseRepr + "*";
            }
            else if (tag == llvm::dwarf::DW_TAG_const_type)
            {
                // Handle const-types
                auto baseRepr = getSrcReprOfType(baseType);
                return addConst(baseRepr);
                // return getSrcReprOfType(baseType) + " const ";
            }
            else
            {
                throw utils::PallasException(
                    "Encountered an unsupported type-tag: " + dType->getTag());
            }
        }

        // Assume that the DIType is a plain DIType and just get the name
        auto typeName = type->getName().str();
        if (typeName.empty())
        {
            throw utils::PallasException(
                "Failed to generate string-representation for type.");
        }

        // Recursively construct the namespace
        auto nSpaceStr = getSrcNamespace(type->getScope());

        return nSpaceStr + typeName;
    }

    std::string SourceWrapperGenerator::getSrcNamespace(llvm::DIScope *scope)
    {
        auto nSpace = llvm::dyn_cast_or_null<llvm::DINamespace>(scope);
        if (nSpace == nullptr)
            return "";
        auto currentNSpace = nSpace->getName().str();
        auto parentNSpace = getSrcNamespace(nSpace->getScope());
        return addNamespace(parentNSpace, currentNSpace);
    }
}