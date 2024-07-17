#include "CWrapperGenerator.h"
#include "SrcLookup.h"
#include "PallasException.h"

#include <sstream>

namespace sourcespec2ir
{
    std::string CWrapperGenerator::getWrapperFunctionString(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const std::string &expr,
        const std::string &functionName)
    {
        std::stringstream stream;

        // Use #pragma to enforce warnings being treated as errors
        // in wrapper-functions
        stream << "#pragma clang diagnostic push \n";
        stream << "#pragma clang diagnostic error \"-Wall\" \n";

        stream << "bool " << functionName << " (";

        // Argument-list
        for (auto it = srcVars.begin(), end = srcVars.end(); it != end; ++it)
        {
            if (it != srcVars.begin())
                stream << ", ";
            const auto varName = utils::getSrcVarName(**it);
            const auto varType = getSrcReprOfType((*it)->getType());
            stream << varType << " " << varName;
        }
        stream << ")";

        // Construct body:
        stream << "{ \n";
        stream << "return " << expr << "; \n";
        stream << "} \n";

        stream << "#pragma clang diagnostic pop \n";

        return stream.str();
    }

    std::string CWrapperGenerator::addConst(const std::string &baseType)
    {
        return baseType + " const ";
    }

    std::string CWrapperGenerator::addPointer(const std::string &baseType)
    {
        return baseType + "*";
    }

    std::string CWrapperGenerator::addTypedef(
        const std::string &baseType, const std::string &typedefName)
    {
        return typedefName;
    }
    
    void CWrapperGenerator::modifySrcWithWrappers(std::vector<SrcLine> &srcLines)
    {
        // Add include for the specification-library to the top of the
        // generated file.
        srcLines.insert(srcLines.begin(),
                        SrcLine("#include <c_spec_lib.h>", 0));
    }
}