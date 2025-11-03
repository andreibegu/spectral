#include "CWrapperGenerator.h"
#include "SrcLookup.h"

#include <sstream>

namespace sourcespec2ir
{

    CWrapperGenerator::CWrapperGenerator(
        const std::string srcInFile,
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations, 
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers)
        : SourceWrapperGenerator(srcInFile, specDeclarations, wrapperMarkers)
    {
    }

    std::string CWrapperGenerator::getWrapperFunctionString(
        const WrapperSignature &sig,
        const std::string &expr,
        const std::string &functionName)
    {
        std::stringstream stream;

        // Use #pragma to enforce warnings being treated as errors
        // in wrapper-functions
        stream << "#pragma clang diagnostic push \n";
        stream << "#pragma clang diagnostic error \"-Wall\" \n";
        stream << "#pragma clang diagnostic error \"-Wpedantic\" \n";
        stream << "#pragma clang diagnostic ignored \"-Wunused-variable\" \n";

        stream << "bool " << functionName << " (";

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

    std::pair<size_t, size_t> CWrapperGenerator::getOffsetToExprStart(
        const SrcWrapperFunction &wFun)
    {
        return std::make_pair(1, 7);
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

    void CWrapperGenerator::modifySrcWithWrappers(std::list<SrcLine> &srcLines)
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
                        SrcLine("#include <c_spec_lib.h>", 0));
    }
}