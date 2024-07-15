#include "CppWrapperGenerator.h"
#include "PallasException.h"
#include "SrcLookup.h"

#include <sstream>

namespace sourcespec2ir
{
    std::string CppWrapperGenerator::getWrapperFunctionString(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const std::string &expr,
        const std::string &functionName)
    {
        std::stringstream stream;
        stream << " bool " << functionName << "(";

        // Construct arg-list
        for (auto it = srcVars.begin(), end = srcVars.end(); it != end; ++it)
        {
            if (it != srcVars.begin())
                stream << ", ";
            const auto varName = utils::getSrcVarName(**it);
            const auto varType = getSrcReprOfType((*it)->getType());
            stream << varType << " " << varName;
        }
        stream << ")";

        // Construct body
        stream << "{ \n";
        stream << "return " << expr << "; \n";
        stream << "} \n";

        return stream.str();
    }

    std::string CppWrapperGenerator::addConst(const std::string &baseType)
    {
        return baseType + " const ";
    }

    std::string CppWrapperGenerator::addPointer(const std::string &baseType)
    {
        return baseType + "*";
    }

    std::string CppWrapperGenerator::addNamespace(
        const std::string &parentNamespace, const std::string &childNamespace)
    {
        return parentNamespace + childNamespace + "::";
    }

    void CppWrapperGenerator::modifySrcWithWrappers(std::vector<SrcLine> &srcLines)
    {
        // Add include for the specification-library to the top of the 
        // generated file. 
        srcLines.insert(srcLines.begin(),
                        SrcLine("#include \"cpp_spec_lib.h\"", 0));
    }

}