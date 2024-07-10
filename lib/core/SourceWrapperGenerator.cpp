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
                // return baseRepr + "*";
            }
            else if (tag == llvm::dwarf::DW_TAG_const_type)
            {
                // Handle const-types
                auto baseRepr = getSrcReprOfType(baseType);
                return addConst(baseRepr);
                // return getSrcReprOfType(baseType) + " const ";
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

    void SourceWrapperGenerator::buildSrcWithWrappers(
        const std::string &inPath, const std::string &outPath,
        const llvm::ArrayRef<SrcWrapperFunction *> &wrapperFunctions)
    {
        // Load file as vector of lines
        auto sourceLines = readSrcFile(inPath);
        
        // Insert the wrapper-functions 
        for (auto wrapper : wrapperFunctions)
        {
            // Find the index of the line of the insert-position in the vector
            auto currentLine = sourceLines->begin();
            while (currentLine != sourceLines->end() && (currentLine->origLine == 0 ||
                                                         currentLine->origLine < wrapper->insertLoc))
                currentLine++;
            // Move back in the vector to find the first blank line above
            // the insert-position
            while (currentLine != sourceLines->begin() &&
                   !currentLine->content.empty())
                currentLine--;
            // Insert lines of the wrapper-function
            insertLines(*sourceLines, wrapper->stringRepr, currentLine);
        }

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