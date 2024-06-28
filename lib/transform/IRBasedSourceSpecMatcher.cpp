
#include "transform/IRBasedSourceSpecMatcher.h"
#include "utils/PallasException.h"

#include <stdexcept>
#include <string>
#include <utility>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/raw_ostream.h>

namespace
{
    // Returns the given line- and column-info as a string
    std::string getLocationString(size_t startLine, size_t endLine)
    {
        return std::to_string(startLine) + "-" + std::to_string(endLine);
    }

    void printContractMatchingInfo(size_t startLine, size_t endLine,
                                   std::string fName)
    {
        auto locStr = getLocationString(startLine, endLine);
        llvm::outs() << "Matched contract at " << locStr
                     << " to " << fName << "\n";
    }

    void printInvMatchingInfo(size_t specSLine, size_t specELine,
                              size_t loopSLine, size_t loopELine)
    {
        auto specLocStr = getLocationString(specSLine, specELine);
        auto loopLocStr = getLocationString(loopSLine, loopELine);
        llvm::outs() << "Matched invariant at " << specLocStr
                     << " to loop at " << loopLocStr << "\n";
    }

    /**
     * Returns a tuple with the minimum and maximum line-numbers
     * that are covered by the given loop.
     */
    std::pair<unsigned int, unsigned int> getLines(const llvm::Loop &loop)
    {
        auto mdTuple = llvm::dyn_cast_or_null<llvm::MDTuple>(loop.getLoopID());
        if (!mdTuple || mdTuple->getNumOperands() < 3)
        {
            // TODO: Implement fallback-solution by iterating over instructions
            throw sourcespec2ir::utils::PallasException("Failed to get llvm.loop MD-node.");
        }

        // Get the line-info from the llvm.loop md-node.
        auto &startMd = mdTuple->getOperand(1);
        auto &endMd = mdTuple->getOperand(2);

        if (startMd == nullptr || endMd == nullptr)
            throw sourcespec2ir::utils::PallasException("Failed to get dbg location for invariant");

        auto startLoc = llvm::dyn_cast_or_null<llvm::DILocation>(startMd.get());
        auto endLoc = llvm::dyn_cast_or_null<llvm::DILocation>(endMd.get());

        if (startLoc == nullptr || endLoc == nullptr)
            throw sourcespec2ir::utils::PallasException("Failed to get line-info for invariant");

        auto minLine = startLoc->getLine();
        auto maxLine = endLoc->getLine();

        return std::make_pair(minLine, maxLine);
    }

}

namespace sourcespec2ir
{

    IRBasedSourceSpecMatcher::IRBasedSourceSpecMatcher(
        SpecParser::FileContext *specParseTree, llvm::Module *irModule,
        llvm::FunctionAnalysisManager *fam, const bool printMatchingInfo)
        : SourceSpecMatcher(specParseTree, irModule),
          printMatchingInfo(printMatchingInfo), fam(fam)
    {
    }

    std::string IRBasedSourceSpecMatcher::getContractBlockContext(size_t startLine, size_t startCol, size_t endLine, size_t endCol)
    {
        const llvm::Function *closestFunction = nullptr;

        // Set for checking that all functions are declared on distinct lines.
        llvm::SmallSet<unsigned int, 16> lineNumberSet;

        // Get first function under the contract
        for (auto &f : irModule->getFunctionList())
        {
            // Get location-information
            auto diSubprog = f.getSubprogram();
            if (diSubprog != nullptr)
            {
                // Check that all function are declared on distinct lines.
                // (Otherwise this approach does not work)
                auto line = diSubprog->getLine();
                if (lineNumberSet.contains(line))
                {
                    auto lineStr = std::to_string(line);
                    throw utils::PallasException(
                        "Encountered multiple functions with the same line-information (" + lineStr + ")");
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
                    throw utils::PallasException("Unable to determine line-info for function " + fName);
                }
            }
        }

        if (closestFunction == nullptr)
        {
            auto locStr = std::to_string(startLine) + ":" + std::to_string(startCol) + "-" +
                          std::to_string(endLine) + ":" + std::to_string(endCol);
            throw utils::PallasException("Unable to match contract (" + locStr + ") to function");
        }

        // Check that no instructions are between the specification and the function declaration.
        // TODO: Is this strictly nessessary? Based on the above algorithm, there should never be an
        //       instruction between contract and matched function.
        auto fLine = closestFunction->getSubprogram()->getLine();
        for (auto &f : irModule->getFunctionList())
        {
            for (auto it = inst_begin(f), end = inst_end(f); it != end; ++it)
            {
                auto dbgLoc = it->getDebugLoc();
                // Skip check if no dbg-info is present
                if (!dbgLoc)
                    continue;
                const auto iLine = dbgLoc.getLine();
                if (iLine >= endLine && iLine <= fLine)
                {
                    throw utils::PallasException(
                        "Encountered an instruction between contract and function.");
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
                    throw utils::PallasException(
                        "Encountered a global variable between contract and function.");
                }
            }
        }

        // Print info about matched contract to the console
        if (printMatchingInfo)
        {
            printContractMatchingInfo(startLine, endLine,
                                      closestFunction->getName().str());
        }

        return closestFunction->getName().str();
    }

    std::pair<llvm::Function *, llvm::Loop *> IRBasedSourceSpecMatcher::matchLoopInvBlock(
        size_t startLine, size_t startCol, size_t endLine, size_t endCol)
    {
        // Find the first function above the loop-invariant
        llvm::Function *parentFunction = nullptr;
        unsigned int parentStartLine = 0;
        for (auto &f : *irModule)
        {
            // TODO: Fail if location cannot be determined?
            if (auto subprog = f.getSubprogram())
            {
                // TODO: Determine the end of the scope by iterating over all
                // instruction in the subscopes and comparing the line-numbers.
                auto fLine = subprog->getLine();
                // Skip all functions that start after the spec-block
                if (fLine > startLine)
                    continue;
                if (parentFunction == nullptr || fLine > parentStartLine)
                {
                    // The current funct is below the current parent funct.
                    parentFunction = &f;
                    parentStartLine = fLine;
                }
            }
        }

        if (parentFunction == nullptr ||
            parentFunction->isDeclaration() ||
            parentFunction->isIntrinsic())
        {
            throw utils::PallasException("Failed to determine parent function \
                                    when matching loop-invariant");
        }

        // Find closest loop in the function that is below the spec-block.
        llvm::LoopInfo &li = fam->getResult<llvm::LoopAnalysis>(*parentFunction);
        llvm::Loop *closestLoop = nullptr;
        unsigned int closestLoopStart = 0;
        unsigned int closestLoopEnd = 0;
        // for (auto *loop : li)
        for (auto *loop : li.getLoopsInPreorder())
        {
            auto [loopStart, loopEnd] = getLines(*loop);
            // Skip all loops that occur before the spec-block
            if (loopStart < startLine)
                continue;
            if (closestLoop == nullptr || loopStart < closestLoopStart)
            {
                // The current loop is below the current closest loop.
                closestLoop = loop;
                closestLoopStart = loopStart;
                closestLoopEnd = loopEnd;
            }
        }

        if (closestLoop == nullptr)
            throw utils::PallasException("Failed to match the loop-invariant");

        if (printMatchingInfo)
        {
            printInvMatchingInfo(startLine, endLine,
                                 closestLoopStart, closestLoopEnd);
        }

        return std::make_pair(parentFunction, closestLoop);
    }
}