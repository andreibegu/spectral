#include "CppWrapperGenerator.h"
#include "PallasException.h"
#include "SrcLookup.h"

#include <llvm/BinaryFormat/Dwarf.h>
#include <sstream>

namespace sourcespec2ir
{
    std::unique_ptr<SrcWrapperFunction> CppWrapperGenerator::getWrappingFunction(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const std::string &expr)
    {

        // Name of wrapping function
        auto funName = "PALLAS_SPEC_" + std::to_string(specFunCnt++);

        std::stringstream stream;
        stream << " bool " << funName << "(";

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

        return std::make_unique<SrcWrapperFunction>(funName, stream.str(), srcVars);
    }

    std::string CppWrapperGenerator::assembleSrcCompilationUnit(
        const SrcWrapperFunction &wrapperFunction,
        const llvm::ArrayRef<std::string> &declarations)
    {
        // TODO: add include-statements from the source-file

        std::stringstream stream;
        stream << "#include \"cpp_spec_lib.h\" \n ";
        // Add any declarations from the declaration-block of the specifications.
        for (auto decl : declarations)
        {
            // TODO: Do not add a ; if the string starts with whitespace
            //       followed by a #
            stream << decl << "; \n ";
        }
        // Add the generated wrapper-function
        stream << wrapperFunction.stringRepr;

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

    std::string CppWrapperGenerator::addNamespace(const std::string &parentNamespace,
                                                  const std::string &childNamespace)
    {
        return parentNamespace + childNamespace + "::";
    }

}