#include "SwiftWrapperGenerator.h"
#include "PallasException.h"
#include "SrcLookup.h"

#include <sstream>

#include <llvm/ADT/SmallSet.h>
#include <llvm/IR/Type.h>

namespace sourcespec2ir
{
    std::string SwiftWrapperGenerator::getWrapperFunctionString(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const std::string &expr,
        const std::string &functionName)
    {

        // Generate the required functions for the specification-library
        const auto specFuncs = generateSpecLibFunctions(srcVars);

        std::stringstream stream;
        stream << "func " << functionName << "(";

        // Construct arg-list
        for (auto it = srcVars.begin(), end = srcVars.end(); it != end; ++it)
        {
            if (it != srcVars.begin())
                stream << ", ";
            const auto varName = utils::getSrcVarName(**it);
            const auto varType = getSrcReprOfType((*it)->getType());
            stream << varName << ": " << varType;
        }
        stream << ") -> Bool ";

        // Construct body
        stream << "{ \n";
        stream << "return " << expr << "; \n";
        stream << "\n";
        stream << specFuncs;
        stream << "} \n";
        stream << "\n";

        return stream.str();
    }

    std::pair<size_t, size_t> SwiftWrapperGenerator::getOffsetToExprStart(
        const SrcWrapperFunction &wFun)
    {
        // TODO: Implement
        return std::make_pair(1, 7);
    }

    std::string SwiftWrapperGenerator::addConst(const std::string &baseType)
    {
        // In swift, function-arguments are const by default
        return baseType;
    }

    std::string SwiftWrapperGenerator::addPointer(const std::string &baseType)
    {
        throw utils::PallasException("Pointers are not yet supported");
    }

    std::string SwiftWrapperGenerator::addReference(const std::string &baseType)
    {
        throw utils::PallasException("References are not yet supported");
    }

    std::string SwiftWrapperGenerator::addTypedef(
        const std::string &baseType, const std::string &typedefName)
    {
        return baseType;
    }

    std::string SwiftWrapperGenerator::addNamespace(
        const std::string &parentNamespace, const std::string &childNamespace)
    {
        throw utils::PallasException("Namespaces are not yet supported");
    }

    void SwiftWrapperGenerator::modifySrcWithWrappers(std::vector<SrcLine> &srcLines)
    {
        // Add include for the specification-module to the top of the
        // generated file.
        srcLines.insert(srcLines.begin(),
                        SrcLine("import PallasSpec", 0));
    }

    std::string SwiftWrapperGenerator::generateSpecLibFunctions(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars)
    {
        // Build a set of the argument-types
        llvm::SmallSet<std::string, 8> types;
        for (auto var : srcVars) 
            types.insert(getSrcReprOfType(var->getType()));

        std::stringstream stream;
        stream << "// START - SpecLib-Functions \n";
        for (auto type : types)
        {
            // Generate function for old
            stream << "@inline(never) \n";
            stream << "func _old(_ v: " << type << ") -> " << type << " { return v; } \n";

            // Generate function for result
            // TODO: We need the return-type here

        }
        stream << "// END - SpecLib-Functions \n";

        return stream.str();
    }

}