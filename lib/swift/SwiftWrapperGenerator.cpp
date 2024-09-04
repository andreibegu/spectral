#include "SwiftWrapperGenerator.h"
#include "PallasException.h"
#include "SrcLookup.h"

#include <sstream>

namespace sourcespec2ir
{
    std::string SwiftWrapperGenerator::getWrapperFunctionString(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const std::string &expr,
        const std::string &functionName)
    {
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
        stream << "} \n";

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

}