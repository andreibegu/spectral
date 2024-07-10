#include "CWrapperGenerator.h"
#include "SrcLookup.h"
#include "PallasException.h"

namespace sourcespec2ir
{
    std::string CWrapperGenerator::getWrapperFunctionString(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const std::string &expr,
        const std::string &functionName)
    {
        // Argument-list
        std::string argList = "";
        for (auto it = srcVars.begin(), end = srcVars.end(); it != end; ++it)
        {
            if (it != srcVars.begin())
            {
                argList += ", ";
            }

            const auto varName = utils::getSrcVarName(**it);
            const auto varType = getSrcReprOfType((*it)->getType());
            argList += varType + " " + varName;
        }

        auto wFun = "bool " + functionName + "(" + argList + ") \n" +
                    "{ \n " +
                    "return " + expr + "; \n" +
                    "} \n";
        return wFun;
    }

    std::string CWrapperGenerator::assembleSrcCompilationUnit(
        const SrcWrapperFunction &wrapperFunction,
        const llvm::ArrayRef<SpecDeclaration> &declarations)
    {
        // TODO: Use a sstream
        // TODO: add include-statements from the source-file
        std::string includes = "#include <c_spec_lib.h> \n ";

        // Add any declarations that were defined in the declaration-block of the specifications.
        for (auto decl : declarations)
        {
            includes += (decl.content + "; \n ");
        }

        return includes + wrapperFunction.stringRepr;
    }

    std::string CWrapperGenerator::addConst(const std::string &baseType)
    {
        return baseType + " const ";
    }

    std::string CWrapperGenerator::addPointer(const std::string &baseType)
    {
        return baseType + "*";
    }

    std::string CWrapperGenerator::addNamespace(const std::string &parentNamespace,
                                                const std::string &childNamespace)
    {
        // C does not have namespaces --> this should never happen
        throw utils::PallasException("Namespaces in C are not supported");
    }

    void CWrapperGenerator::modifySrcWithWrappers(std::vector<SrcLine> &srcLines)
    {
        // Add include for the specification-library to the top of the 
        // generated file. 
        srcLines.insert(srcLines.begin(),
                        SrcLine("#include <c_spec_lib.h>", 0));
    }
}