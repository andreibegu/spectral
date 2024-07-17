#include "SourceWrapperGenerator.h"
#include "PallasException.h"

#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <llvm/BinaryFormat/Dwarf.h>

namespace sourcespec2ir
{

    std::string SourceWrapperGenerator::getSrcReprOfType(const llvm::DIType *type)
    {
        // Check if type is a derived type
        if (auto *dType = llvm::dyn_cast<llvm::DIDerivedType>(type))
        {
            auto tag = dType->getTag();
            auto baseType = dType->getBaseType();

            if (tag == llvm::dwarf::DW_TAG_pointer_type)
            {
                // Handle Pointer-type
                // Recursively unwrap the pointer
                auto baseRepr = getSrcReprOfType(baseType);
                return addPointer(baseRepr);
            }
            else if (tag == llvm::dwarf::DW_TAG_const_type)
            {
                // Handle const-types
                auto baseRepr = getSrcReprOfType(baseType);
                return addConst(baseRepr);
            }
            else if (tag == llvm::dwarf::DW_TAG_reference_type)
            {
                // Handle references
                return addReference(getSrcReprOfType(baseType));
            }
            else if (tag == llvm::dwarf::DW_TAG_typedef)
            {
                // Handle typedefs
                auto typedefName = dType->getName().str();
                return addTypedef(getSrcReprOfType(baseType), typedefName);
            }
            else
            {
                throw utils::PallasException(
                    "Encountered an unsupported type-tag: " + dType->getTag());
            }
        }

        // Assume that the DIType is a plain DIType and just get the name
        auto typeName = type->getName().str();
        if (typeName.empty())
        {
            throw utils::PallasException(
                "Failed to generate string-representation for type.");
        }

        // Recursively construct the namespace
        auto nSpaceStr = getSrcNamespace(type->getScope());

        return nSpaceStr + typeName;
    }

    std::string SourceWrapperGenerator::getSrcNamespace(llvm::DIScope *scope)
    {
        auto nSpace = llvm::dyn_cast_or_null<llvm::DINamespace>(scope);
        if (nSpace == nullptr)
            return "";
        auto currentNSpace = nSpace->getName().str();
        auto parentNSpace = getSrcNamespace(nSpace->getScope());
        return addNamespace(parentNSpace, currentNSpace);
    }

    std::string SourceWrapperGenerator::addReference(const std::string &baseType)
    {
        // Overwrite this in a subclass to support references.
        throw utils::PallasException(
            "References are not supported by this WrapperGenerator.");
    }

    std::string SourceWrapperGenerator::addTypedef(
        const std::string &baseType, const std::string &typedefName)
    {
        // Overwrite this in a subclass to support typedefs.
        throw utils::PallasException(
            "Typedefs are not supported by this WrapperGenerator.");
    }

    size_t SourceWrapperGenerator::getWrapperInsertLoc(
        const MatchedFunctionContract &contract)
    {
        return contract.function->getSubprogram()->getLine();
    }

    size_t SourceWrapperGenerator::getWrapperInsertLoc(
        const MatchedLoopInvariantBlock &invBlock)
    {
        return invBlock.parentFunction->getSubprogram()->getLine();
    }

    std::string SourceWrapperGenerator::getNameForWrapperFunction()
    {
        return "PALLAS_SPEC_" + std::to_string(specFunCnt++);
    }

    void SourceWrapperGenerator::modifySrcWithWrappers(
        std::vector<SrcLine> &srcLines)
    {
        // Do nothing. Subclasses can overwrite this
    }

    std::vector<SourceWrapperGenerator::SrcLine>::iterator
    SourceWrapperGenerator::findInsertPos(
        const size_t srcLineNumber, std::vector<SrcLine> &srcLines)
    {
        // Find the index of the line of the insert-position in the vector
        auto currentLine = srcLines.begin();
        while (currentLine != srcLines.end() && (currentLine->origLine == 0 ||
                                                 currentLine->origLine < srcLineNumber))
            currentLine++;
        // Move back in the vector to find the first blank line above
        // the insert-position
        while (currentLine != srcLines.begin() &&
               !currentLine->content.empty())
            currentLine--;

        return currentLine;
    }

    void SourceWrapperGenerator::buildSrcWithWrappers(
        const std::string &inPath, const std::string &outPath,
        llvm::ArrayRef<SrcWrapperFunction *> wrapperFunctions,
        llvm::ArrayRef<SpecDeclaration> specDeclarations)
    {
        // Load file as vector of lines
        auto sourceLines = readSrcFile(inPath);

        // Insert contents of the declaration-block
        for (auto &decl : specDeclarations)
        {
            auto declStr = decl.content + ";";
            auto insertPos = findInsertPos(decl.srcFileLine, *sourceLines);
            insertLines(*sourceLines, declStr, insertPos);
        }

        // Insert the wrapper-functions
        for (auto wrapper : wrapperFunctions)
        {
            // Insert lines of the wrapper-function
            auto insertPos = findInsertPos(wrapper->insertLoc, *sourceLines);
            insertLines(*sourceLines, wrapper->stringRepr, insertPos);
        }

        // Call entry-point for subclasses to customize the generated source.
        modifySrcWithWrappers(*sourceLines);

        // Write modified file
        writeSrcFile(outPath, *sourceLines);
    }

    std::unique_ptr<std::vector<SourceWrapperGenerator::SrcLine>>
    SourceWrapperGenerator::readSrcFile(std::string filePath)
    {
        std::ifstream inStream(filePath);
        if (!inStream.is_open())
            throw utils::PallasException("Failed to read source-file during wrapping-stage");

        auto lineVec = std::make_unique<std::vector<SrcLine>>();
        size_t currentLineNo = 1;
        std::string currentLine = "";
        while (getline(inStream, currentLine))
        {
            lineVec->push_back(SrcLine(currentLine, currentLineNo));
            currentLineNo++;
        }
        inStream.close();
        return lineVec;
    }

    void SourceWrapperGenerator::writeSrcFile(const std::string &filePath,
                                              const std::vector<SrcLine> &lines)
    {
        std::ofstream outStream(filePath);
        if (!outStream.is_open())
            throw utils::PallasException("Failed to write source with wrappers to file.");

        for (auto line : lines)
            outStream << line.content << "\n";

        outStream.close();
    }

    void SourceWrapperGenerator::insertLines(
        std::vector<SrcLine> &lines, const std::string &content,
        std::vector<SrcLine>::iterator insertLine)
    {
        // Construct vector that contains the lines of the content
        std::stringstream contentStream(content);
        std::string currentCLine;
        std::vector<SrcLine> contentVec;
        contentVec.push_back(SrcLine("", 0));
        while (std::getline(contentStream, currentCLine))
            contentVec.push_back(SrcLine(currentCLine, 0));
        // Insert content of vector
        lines.insert(insertLine, contentVec.begin(), contentVec.end());
    }
}