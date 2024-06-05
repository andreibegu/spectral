
#include "transform/IRBasedSourceSpecMatcher.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InstIterator.h>

namespace
{
    void printMatchingInfo(size_t startLine, size_t startCol, size_t endLine, size_t endCol, std::string fName)
    {
        auto locStr = std::to_string(startLine) + ":" + std::to_string(startCol) + "-" +
                      std::to_string(endLine) + ":" + std::to_string(endCol);
        std::cout << "Matched contract at " << locStr << " to " << fName << std::endl;
    }
}

namespace sourcespec2ir
{

    IRBasedSourceSpecMatcher::IRBasedSourceSpecMatcher(SpecParser::FileContext *specParseTree, llvm::Module *irModule)
        : SourceSpecMatcher(specParseTree, irModule)
    {
    }

    std::string IRBasedSourceSpecMatcher::getContractBlockContext(size_t startLine, size_t startCol, size_t endLine, size_t endCol)
    {
        const llvm::Function *closestFunction = nullptr;

        // Set that is used to check that all functions are declared on distinct lines.
        llvm::SmallSet<unsigned int, 16> lineNumberSet;

        // Get first function under the contract
        for (auto &f : irModule->getFunctionList())
        {
            // Get location-information
            auto diSubprog = f.getSubprogram();
            if (diSubprog != nullptr)
            {
                // Check that all function are declared on distinct lines. (Otherwise this approach does not work)
                auto line = diSubprog->getLine();
                if (lineNumberSet.contains(line))
                {
                    auto lineStr = std::to_string(line);
                    throw std::runtime_error("Encountered multiple functions with the same line-information (" + lineStr + ")");
                }
                lineNumberSet.insert(line);

                // Check if function occurs after contract and is closer than current closest function.
                if (line > endLine && (closestFunction == nullptr || line < closestFunction->getSubprogram()->getLine()))
                {
                    closestFunction = &f;
                }

                // TODO: Factor in parent-scope/ general scope info when determining line-info
                //       (I.e. what if file was linked form multiple scopes?)
            }
            else
            {
                // Check if the function is an intrinsic. If so, this is fine. Otherwise fail?!
                if (!f.isIntrinsic())
                {
                    auto fName = f.getName().str();
                    throw std::runtime_error("Unable to determine line-info for function " + fName);
                }
            }
        }

        if (closestFunction == nullptr)
        {
            auto locStr = std::to_string(startLine) + ":" + std::to_string(startCol) + "-" +
                          std::to_string(endLine) + ":" + std::to_string(endCol);
            throw std::runtime_error("Unable to match contract (" + locStr + ") to function");
        }

        // Check that no instructions are between the specification and the function declaration.
        // TODO: Is this strictly nessessary? Based on the above algorithm, there should never be an
        //       instruction between contract and matched function.
        auto fLine = closestFunction->getSubprogram()->getLine();
        for (auto &f : irModule->getFunctionList())
        {
            for (auto it = inst_begin(f), end = inst_end(f); it != end; ++it)
            {
                const auto iLine = it->getDebugLoc().getLine();
                if (iLine >= endLine && iLine <= fLine)
                {
                    throw std::runtime_error("Encountered an instruction between contract and function.");
                }
            }
        }
        // Check that no global variables are between the specification and the function declaration.
        for (auto &g : irModule->getGlobalList())
        {
            llvm::SmallVector<llvm::DIGlobalVariableExpression *> dbgExpressions;
            g.getDebugInfo(dbgExpressions);
            for (auto dbgExpr : dbgExpressions)
            {
                const auto gLine = dbgExpr->getVariable()->getLine();
                if (gLine >= endLine && gLine <= fLine)
                {
                    throw std::runtime_error("Encountered a global variable between contract and function.");
                }
            }
        }

        // Print info about matched contract to the console
        printMatchingInfo(startLine, startCol, endLine, endCol, closestFunction->getName().str());

        return closestFunction->getName().str();
    }
}