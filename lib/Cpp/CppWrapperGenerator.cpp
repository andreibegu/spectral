#include "CppWrapperGenerator.h"
#include "SrcLookup.h"

#include <sstream>

namespace sourcespec2ir
{
    CppWrapperGenerator::CppWrapperGenerator(
        const std::string srcInFile,
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations,
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers)
        : SourceWrapperGenerator(srcInFile, specDeclarations, wrapperMarkers)
    {
    }

    std::string CppWrapperGenerator::getWrapperFunctionString(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const std::string &expr,
        const std::string &functionName,
        const llvm::DIType *returnType)
    {
        std::stringstream stream;
        stream << "__attribute__((used)) \n";
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

    std::pair<size_t, size_t> CppWrapperGenerator::getOffsetToExprStart(
        const SrcWrapperFunction &wFun)
    {
        return std::make_pair(1, 7);
    }

    std::string CppWrapperGenerator::addConst(const std::string &baseType)
    {
        return baseType + " const ";
    }

    std::string CppWrapperGenerator::addPointer(const std::string &baseType)
    {
        return baseType + "*";
    }

    std::string CppWrapperGenerator::addReference(const std::string &baseType)
    {
        return baseType + " &";
    }

    std::string CppWrapperGenerator::addTypedef(
        const std::string &baseType, const std::string &typedefName)
    {
        return typedefName;
    }

    std::string CppWrapperGenerator::addNamespace(
        const std::string &parentNamespace, const std::string &childNamespace)
    {
        return parentNamespace + childNamespace + "::";
    }

    void CppWrapperGenerator::modifySrcWithWrappers(std::vector<SrcLine> &srcLines)
    {

        // Add content of the declaration-block to the file.
        for (auto &decl : specDeclarations)
        {
            auto declStr = decl.content + ";";
            auto insertPos = findInsertPos(decl.srcFileLine, srcLines);
            insertLines(srcLines, declStr, insertPos);
        }

        // Add include for the specification-library to the top of the
        // generated file.
        srcLines.insert(srcLines.begin(),
                        SrcLine("#include \"cpp_spec_lib.h\"", 0));
    }

}