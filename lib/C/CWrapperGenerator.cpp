#include "CWrapperGenerator.h"
#include "utils/PallasException.h"
#include "utils/SrcLookup.h"

#include <llvm/BinaryFormat/Dwarf.h>

namespace sourcespec2ir
{
    std::unique_ptr<SrcWrapperFunction> CWrapperGenerator::getWrappingFunction(
        llvm::ArrayRef<llvm::DILocalVariable *> srcVars, const std::string &expr)
    {

        // Name of wrapping function
        auto funName = "PALLAS_SPEC_" + std::to_string(specFunCnt++);

        // Argument-list
        std::string argList = "";
        for (auto it = srcVars.begin(), end = srcVars.end(); it != end; ++it)
        {
            if (it != srcVars.begin())
            {
                argList += ", ";
            }

            const auto varName = utils::getSrcVarName(**it);
            const auto varType = getSrcReprOfType((*it)->getType());
            argList += varType + " " + varName;
        }

        auto wFun = "bool " + funName + "(" + argList + ") \n" +
                    "{ \n " +
                    "return " + expr + "; \n" +
                    "} \n";

        return std::make_unique<SrcWrapperFunction>(funName, wFun);
    }

    std::string CWrapperGenerator::assembleSrcCompilationUnit(
        const SrcWrapperFunction &wrapperFunction,
        const llvm::ArrayRef<std::string> &declarations)
    {
        // TODO: Use a sstream
        // TODO: add include-statements from the source-file
        std::string includes = "#include <c_spec_lib.h> \n ";

        // Add any declarations that were defined in the declaration-block of the specifications.
        for (auto decl : declarations)
        {
            includes += (decl + "; \n ");
        }

        return includes + wrapperFunction.stringRepr;
    }

    /**
     * TODO: Move source-language independent aspects into superclass.
     * TODO: Support more Tags
     */
    std::string CWrapperGenerator::getSrcReprOfType(const llvm::DIType *type)
    {
        // Check if type is a pointer:
        if (auto *dType = llvm::dyn_cast<llvm::DIDerivedType>(type))
        {
            auto tag = dType->getTag();
            auto baseType = dType->getBaseType();

            if (tag == llvm::dwarf::DW_TAG_pointer_type)
            {
                // Handle Pointer-type
                // Recursively unwrap the pointer
                auto baseRepr = getSrcReprOfType(baseType);
                return baseRepr + "*";
            }
            else if (tag == llvm::dwarf::DW_TAG_const_type)
            {
                // Handle const-types
                return getSrcReprOfType(baseType) + " const ";
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
        return typeName;
    }

}