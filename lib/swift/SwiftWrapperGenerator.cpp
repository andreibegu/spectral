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
        const std::string &functionName,
        const llvm::DIType *returnType)
    {

        // Generate the required functions for the specification-library
        const auto specFuncs = generateSpecLibFunctions(srcVars, returnType);

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

    std::string SwiftWrapperGenerator::getSrcReprOfType(
        const llvm::DIType *type)
    {
        // Get src-representation of the type using default-implementation
        const auto srcType = SourceWrapperGenerator::getSrcReprOfType(type);

        // Manually handle some cases where the type is not correctly resolved.
        if (srcType == "$sytD")
            return "Void";

        return srcType;
    }

    std::pair<size_t, size_t> SwiftWrapperGenerator::getOffsetToExprStart(
        const SrcWrapperFunction &wFun)
    {
        return std::make_pair(1, 7);
    }

    std::string SwiftWrapperGenerator::addConst(const std::string &baseType)
    {
        // In swift, function-arguments are const by default
        return baseType;
    }

    std::string SwiftWrapperGenerator::addTypedef(
        const std::string &baseType, const std::string &typedefName)
    {
        return baseType;
    }

    void SwiftWrapperGenerator::modifySrcWithWrappers(std::vector<SrcLine> &srcLines)
    {
        // Add include for the specification-module to the top of the
        // generated file.
        srcLines.insert(srcLines.begin(),
                        SrcLine("import PallasSpec", 0));
    }

    std::string SwiftWrapperGenerator::generateSpecLibFunctions(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const llvm::DIType *returnType)
    {
        std::stringstream stream;
        stream << "// START - SpecLib-Functions \n";

        // Generate function for result
        if (returnType != nullptr)
        {
            // The spec-function is only a placeholder and not executed.
            // So we just throw a fatal error to get it to compile.
            const auto retTypeStr = getSrcReprOfType(returnType);
            if (retTypeStr != "Void")
            {
                stream << "@inline(never) \n";
                stream << "func _result() -> " << retTypeStr << " { "
                       << "fatalError(\"Spec-placeholder may not be executed.\"); "
                       << "} \n";
            }
        }

        // Build a set of the argument-types to avoid duplicates
        llvm::SmallSet<std::string, 8> types;
        for (auto var : srcVars)
            types.insert(getSrcReprOfType(var->getType()));
        for (auto type : types)
        {
            // Generate function for old
            stream << "@inline(never) \n";
            stream << "func _old(_ v: " << type << ") -> " << type
                   << " { return v; } \n";
            // Generate function for Perm
            stream << "@inline(never) \n";
            stream << "func _perm(_ loc: " << type << ", _ f: Frac) -> Bool "
                   << "{ return true; } \n";
        }
        stream << "// END - SpecLib-Functions \n";
        return stream.str();
    }

}