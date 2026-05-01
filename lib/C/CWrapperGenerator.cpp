#include "CWrapperGenerator.h"
#include "PallasException.h"

#include <sstream>

namespace sourcespec2ir
{

    CWrapperGenerator::CWrapperGenerator(
        const std::string srcInFile,
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations,
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers,
        std::unique_ptr<SourceSyntaxResolver> syntaxResolver 
    )
        : SourceWrapperGenerator(
            srcInFile, specDeclarations, wrapperMarkers, std::move(syntaxResolver))
    {
    }

    std::string CWrapperGenerator::getWrapperFunctionString(
        const WrapperSignature &sig,
        const std::string &expr,
        const std::string &functionName)
    {
        std::stringstream stream;
        auto rType = sig.returnTy.has_value()
                         ? sig.returnTy.value().prefix.str().str()
                         : "bool";

        // Use #pragma to enforce warnings being treated as errors
        // in wrapper-functions
        stream << "#pragma clang diagnostic push \n";
        stream << "#pragma clang diagnostic error \"-Wall\" \n";
        stream << "#pragma clang diagnostic error \"-Wpedantic\" \n";
        stream << "#pragma clang diagnostic ignored \"-Wunused-variable\" \n";

        stream << rType << " " << functionName << " (";

        // Argument-list
        if (sig.args.empty())
        {
            stream << "void";
        }
        else
        {
            bool isFirst = true;
            for (auto &arg : sig.args)
            {
                if (!isFirst)
                    stream << ", ";
                stream << arg;
                isFirst = false;
            }
        }

        stream << ")";

        // Construct body:
        stream << "{ \n";
        stream << "return " << expr << "; \n";
        stream << "} \n";

        stream << "#pragma clang diagnostic pop \n";

        return stream.str();
    }

    std::string CWrapperGenerator::getPredicateDefString(
        const std::string &name, const std::string &args,
        const std::string &body)
    {
        std::stringstream s;
        s << "#pragma clang diagnostic push \n"
          << "#pragma clang diagnostic error \"-Wall\" \n"
          << "#pragma clang diagnostic error \"-Wpedantic\" \n"
          << "#pragma clang diagnostic ignored \"-Wunused-variable\" \n"
          << "#pragma clang diagnostic ignored \"-Winfinite-recursion\" \n";

        s << "bool " << name << "(" << args << ") { \n"
          << "return " << body << "; \n"
          << "} \n";

        s << "#pragma clang diagnostic pop \n";

        return s.str();
    }

    std::pair<size_t, size_t> CWrapperGenerator::getOffsetToExprStart(
        const SrcWrapperFunction &wFun)
    {
        return std::make_pair(1, 7);
    }

    std::pair<size_t, size_t> CWrapperGenerator::getOffsetToExprStart(
        const SpecPredicate &pDef)
    {
        return std::make_pair(1, 7);
    }

    SrcTypeRepresentation CWrapperGenerator::getVoid()
    {
        return SrcTypeRepresentation("void");
    }

    void CWrapperGenerator::addConst(SrcTypeRepresentation &baseType)
    {
        baseType.prefix.append(" const ");
    }

    void CWrapperGenerator::addPointer(SrcTypeRepresentation &baseType)
    {
        baseType.prefix.append("*");
    }

    void CWrapperGenerator::addTypedef(SrcTypeRepresentation &baseType,
                                       const std::string &typedefName)
    {
        baseType.prefix = typedefName;
        baseType.suffix = "";
    }

    void CWrapperGenerator::addArrayDim(SrcTypeRepresentation &baseType,
                                        const std::string &dimSize)
    {
        baseType.suffix.append("[" + dimSize + "]");
    }

    void CWrapperGenerator::modifySrcWithWrappers(std::list<SrcLine> &srcLines)
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
                        SrcLine("#include <c_spec_lib.h>", 0));
    }
}