#include "CppWrapperGenerator.h"
#include "PallasException.h"

#include <sstream>

namespace sourcespec2ir
{
    CppWrapperGenerator::CppWrapperGenerator(
        const std::string srcInFile,
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations,
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers, 
        std::unique_ptr<SourceSyntaxResolver> syntaxResolver)
        : SourceWrapperGenerator(
            srcInFile, specDeclarations, wrapperMarkers, std::move(syntaxResolver))
    {
    }

    std::string CppWrapperGenerator::getWrapperFunctionString(
        const WrapperSignature &sig,
        const std::string &expr,
        const std::string &functionName)
    {
        auto rType = sig.returnTy.has_value()
                         ? sig.returnTy.value().prefix.str().str()
                         : "bool";
        std::stringstream stream;

        stream << "#pragma clang diagnostic push \n";
        stream << "#pragma clang diagnostic error \"-Wall\" \n";
        stream << "#pragma clang diagnostic error \"-Wpedantic\" \n";
        stream << "#pragma clang diagnostic ignored \"-Wunused-variable\" \n";

        stream << "__attribute__((used)) \n";
        stream << " " << rType << " " << functionName << "(";

        // Construct arg-list
        bool firstArg = true;
        for (auto &arg : sig.args)
        // for (auto it = srcVars.begin(), end = srcVars.end(); it != end; ++it)
        {
            // Skip the 'this' argument as it gets added implicitly
            if (arg.ends_with(" this"))
                continue;
            if (!firstArg)
                stream << ", ";
            stream << arg;
            firstArg = false;
        }
        stream << ")";

        // Construct body
        stream << "{ \n";
        stream << "return " << expr << "; \n";
        stream << "} \n";

        stream << "#pragma clang diagnostic pop \n";

        return stream.str();
    }

    std::string CppWrapperGenerator::getPredicateDefString(
        const std::string &name, const std::string &args,
        const std::string &body)
    {
        std::stringstream s;
        s << "#pragma clang diagnostic push \n"
          << "#pragma clang diagnostic error \"-Wall\" \n"
          << "#pragma clang diagnostic error \"-Wpedantic\" \n"
          << "#pragma clang diagnostic ignored \"-Wunused-variable\" \n"
          << "__attribute__((used)) \n";

        s << "bool " << name << "(" << args << ") { \n"
          << "return " << body << "; \n"
          << "} \n";

        s << "#pragma clang diagnostic pop \n";

        return s.str();
    }

    std::pair<size_t, size_t> CppWrapperGenerator::getOffsetToExprStart(
        const SrcWrapperFunction &wFun)
    {
        return std::make_pair(1, 7);
    }

    std::pair<size_t, size_t> CppWrapperGenerator::getOffsetToExprStart(
        const SpecPredicate &pDef)
    {
        return std::make_pair(1, 7);
    }

    SrcTypeRepresentation CppWrapperGenerator::getVoid()
    {
        return SrcTypeRepresentation("void");
    }

    void CppWrapperGenerator::addConst(SrcTypeRepresentation &baseType)
    {
        baseType.prefix.append(" const ");
    }

    void CppWrapperGenerator::addPointer(SrcTypeRepresentation &baseType)
    {
        baseType.prefix.append("*");
    }

    void CppWrapperGenerator::addReference(SrcTypeRepresentation &baseType)
    {
        baseType.prefix.append(" &");
    }

    void CppWrapperGenerator::addTypedef(
        SrcTypeRepresentation &baseType, const std::string &typedefName)
    {
        baseType.prefix = typedefName;
        baseType.suffix = "";
    }

    void CppWrapperGenerator::addArrayDim(SrcTypeRepresentation &baseType,
                                          const std::string &dimSize)
    {
        baseType.suffix.append("[" + dimSize + "]");
    }

    std::string CppWrapperGenerator::addNamespace(
        const std::string &parentNamespace, const std::string &childNamespace)
    {
        return parentNamespace + childNamespace + "::";
    }

    void CppWrapperGenerator::modifySrcWithWrappers(std::list<SrcLine> &srcLines)
    {

        // Add content of the declaration-block to the file.
        for (auto &decl : specDeclarations)
        {
            auto declStr = decl.content + ";";
            auto insertPos = findInsertPos(decl.srcLoc.getStartLine(), srcLines);
            insertLines(srcLines, declStr, insertPos);
        }

        // Add include for the specification-library to the top of the
        // generated file.
        srcLines.insert(srcLines.begin(),
                        SrcLine("#include \"cpp_spec_lib.h\"", 0));
    }
}