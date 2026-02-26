
#include "IRBasedSourceSpecMatcher.h"
#include "PallasException.h"
#include "SrcLookup.h"
#include "FileUtils.h"
#include "IRUtils.h"

#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <map>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/raw_ostream.h>

namespace
{
    void printContractMatchingInfo(const sourcespec2ir::SrcRange &specLoc,
                                   std::string fName)
    {
        llvm::outs() << "Matched contract at " << specLoc.asShortString()
                     << " to " << fName << "\n";
    }

    void printExternalContractMatchingInfo(
        const sourcespec2ir::SrcRange &specLoc,
        std::string fName, bool isGenerated)
    {
        auto cType = isGenerated ? "generated" : "external";
        llvm::outs() << "Matched " << cType << " contract at "
                     << specLoc.asShortString()
                     << " to " << fName << "\n";
    }

    void printInvMatchingInfo(const sourcespec2ir::SrcRange &specLoc,
                              size_t loopSLine)
    {
        llvm::outs() << "Matched invariant at " << specLoc.asShortString()
                     << " to loop at " << std::to_string(loopSLine) << "\n";
    }

    std::string getLocString(const llvm::DebugLoc &dbgLoc)
    {
        return std::to_string(dbgLoc.getLine()) + "::" +
               std::to_string(dbgLoc.getCol());
    }

    void printStmntBlockMatchingInfo(const sourcespec2ir::SrcRange &specLoc,
                                     const llvm::Instruction *instr)
    {
        auto specLocStr = specLoc.asShortString();
        auto instLocStr = getLocString(instr->getDebugLoc());
        llvm::outs() << "Matched statement-block at " << specLocStr
                     << " to instruction at " << instLocStr << "\n";
    }

    void printGhostArgBindingMatchingInfo(sourcespec2ir::SrcRange specLoc,
                                          const llvm::CallInst *callInst)
    {
        llvm::outs() << "Matched ghost argument binding at "
                     << specLoc.asShortString(false)
                     << " to call at "
                     << getLocString(callInst->getDebugLoc())
                     << "\n";
    }

    /**
     * Returns a tuple with the minimum and maximum line-numbers
     * that are covered by the given loop.
     */
    std::pair<unsigned int, unsigned int> getLines(const llvm::Loop &loop)
    {
        auto loopRange = loop.getLocRange();
        auto &startLoc = loopRange.getStart();
        auto &endLoc = loopRange.getEnd();

        if (!startLoc || !endLoc)
            throw sourcespec2ir::utils::PallasException(
                "Failed to get line-info for invariant");

        auto minLine = startLoc.getLine();
        auto maxLine = endLoc.getLine();
        return std::make_pair(minLine, maxLine);
    }

}

namespace sourcespec2ir
{

    IRBasedSourceSpecMatcher::IRBasedSourceSpecMatcher(
        SpecParser::FileContext *specParseTree,
        const std::string &srcFilePath, llvm::Module *irModule,
        llvm::FunctionAnalysisManager *fam, const bool printMatchingInfo,
        bool matchExternalOnly, std::unique_ptr<SourceSyntaxResolver> syntaxResolver)
        : SourceSpecMatcher(specParseTree, srcFilePath,
                            irModule, printMatchingInfo, matchExternalOnly,
                            std::move(syntaxResolver)),
          fam(fam)
    {
    }

    llvm::Function *IRBasedSourceSpecMatcher::matchFContract(SrcRange loc)
    {
        llvm::Function *closestFunction = nullptr;
        auto endLine = loc.getEndLine();

        // Map of line-numbers to functions to check for ambiguous matching
        std::map<unsigned int, llvm::SmallPtrSet<llvm::Function *, 4>> lineToFunc;

        // Get first function under the contract
        for (auto &f : irModule->getFunctionList())
        {
            // Get location-information
            auto diSubprog = utils::getDeclSubprog(f);
            if (diSubprog != nullptr)
            {
                if (!isFromMainFile(*diSubprog))
                    continue;
                // If the function is an artificially generated one, skip it.
                // This is e.g. the case for the default-arguments in swift.
                if (isArtificial(diSubprog))
                    continue;

                auto line = diSubprog->getLine();
                // Bookkeeping of which functions occur on this line
                if (lineToFunc.find(line) != lineToFunc.end())
                    lineToFunc[line].insert(&f);
                else
                    lineToFunc.insert({line, {&f}});

                // Check if function occurs after contract and is closer than current closest function.
                if (line > endLine &&
                    (closestFunction == nullptr || line < utils::getDeclSubprog(*closestFunction)->getLine()))
                {
                    closestFunction = &f;
                }

                // TODO: Factor in parent-scope/ general scope info when determining line-info
                //       (I.e. what if file was linked form multiple scopes?)
            }
            else
            {
                // If we cannot determine the line for a function, we issue
                // a warning. This is not directly an error but could lead to
                // incorrectly matched contracts if the line-info for a relevant
                // function is missing
                if (!f.isIntrinsic() && !f.isDeclaration() && printMatchingInfo)
                {
                    llvm::outs()
                        << "WARNING: Unable to determine line for function "
                        << f.getName().str() << "\n";
                }
            }
        }

        if (closestFunction == nullptr)
        {
            auto locStr = std::to_string(loc.getStartLine()) + ":" +
                          std::to_string(loc.getStartCol()) + "-" +
                          std::to_string(loc.getEndLine()) + ":" +
                          std::to_string(loc.getEndCol());
            throw utils::PallasException(
                "Unable to match contract (" + locStr + ") to function");
        }

        // Check that the closest function does not occur on the same line as
        // other functions. In this case, the matching would be ambiguous
        auto closestLine = utils::getDeclSubprog(*closestFunction)->getLine();
        if (lineToFunc[closestLine].size() > 1)
        {
            throw utils::PallasException(
                "Matching failed: Multiple functions occur on the same line (" +
                std::to_string(closestLine) + ")");
        }

        // Check that no instructions occur between the specification and the
        // function declaration.
        // TODO: Is this strictly nessessary? Based on the above algorithm,
        // there should never be an instruction between contract and matched
        // function.
        // auto fLine = getSProgForFunc(*closestFunction)->getLine();
        for (auto &f : irModule->getFunctionList())
        {
            // Skip artificial functions
            auto sProg = utils::getDeclSubprog(f);
            if (sProg != nullptr && isArtificial(sProg))
                continue;

            for (auto it = inst_begin(f), end = inst_end(f); it != end; ++it)
            {
                auto dbgLoc = it->getDebugLoc();
                // Skip check if no dbg-info is present
                if (!dbgLoc)
                    continue;
                const auto iLine = dbgLoc.getLine();
                auto isDebugIntrinsic = llvm::isa<llvm::DbgVariableIntrinsic>(&*it);
                if (iLine >= endLine && iLine < closestLine)
                {
                    throw utils::PallasException(
                        "Encountered an instruction between contract and function.");
                }
                else if (iLine >= endLine && iLine == closestLine && !isDebugIntrinsic)
                {
                    throw utils::PallasException(
                        "Encountered an instruction on the same line as the function.");
                }
            }
        }
        // Check that no global variables are between the specification and
        // the function declaration.
        for (const auto &g : irModule->globals())
        {
            llvm::SmallVector<llvm::DIGlobalVariableExpression *> dbgExpressions;
            g.getDebugInfo(dbgExpressions);
            for (auto dbgExpr : dbgExpressions)
            {
                const auto gLine = dbgExpr->getVariable()->getLine();
                if (gLine >= endLine && gLine <= closestLine)
                {
                    throw utils::PallasException(
                        "Encountered a global variable between contract and function.");
                }
            }
        }

        // Print info about matched contract to the console
        if (printMatchingInfo)
        {
            printContractMatchingInfo(
                loc, utils::getSrcNameOfFunction(*closestFunction));
        }

        return closestFunction;
    }

    llvm::SmallVector<llvm::Function *>
    IRBasedSourceSpecMatcher::matchFContract(
        const std::string matchingStr, const bool isSrcName, SrcRange loc,
        const bool isGenerated)
    {
        llvm::SmallVector<llvm::Function *> results;
        const std::regex matchRegex(matchingStr);
        for (auto &f : irModule->functions())
        {
            if (isSrcName && f.getSubprogram() == nullptr)
                continue;
            auto name = isSrcName ? f.getSubprogram()->getName().str()
                                  : f.getName().str();

            if (std::regex_match(name, matchRegex))
            {
                results.push_back(&f);
                if (printMatchingInfo)
                    printExternalContractMatchingInfo(
                        loc, utils::getSrcNameOfFunction(f), isGenerated);
            }
        }
        return results;
    }

    std::pair<llvm::Function *, llvm::Loop *>
    IRBasedSourceSpecMatcher::matchLoopInvBlock(SrcRange loc)
    {
        // Find the first function above the loop-invariant
        auto startLine = loc.getStartLine();
        llvm::Function *parentFunction = getFirstFunctionAbove(startLine);

        if (parentFunction == nullptr ||
            parentFunction->isDeclaration() ||
            parentFunction->isIntrinsic())
        {
            throw utils::PallasException(
                "Failed to determine parent function when matching loop-invariant");
        }

        // Find closest loop in the function that is below the spec-block.
        llvm::LoopInfo &li = fam->getResult<llvm::LoopAnalysis>(*parentFunction);
        llvm::Loop *closestLoop = nullptr;
        unsigned int closestLoopStart = 0;
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
            }
        }

        if (closestLoop == nullptr)
            throw utils::PallasException("Failed to match the loop-invariant");

        if (printMatchingInfo)
            printInvMatchingInfo(loc, closestLoopStart);

        return std::make_pair(parentFunction, closestLoop);
    }

    llvm::Instruction *IRBasedSourceSpecMatcher::getClosestInstructionAfter(
        const SrcRange &loc)
    {
        // Get the first function above the spec-block
        // TODO: Consider the range that is covered by the function instead of
        // just considering the starting-line.
        auto startLine = loc.getStartLine();
        auto endLine = loc.getEndLine();
        auto endCol = loc.getEndCol();
        auto parentFunc = getFirstFunctionAbove(startLine);

        if (parentFunc == nullptr)
        {
            std::stringstream s;
            s << "Failed to get enclosing function for specification-block at "
              << loc.asShortString();
            throw utils::PallasException(s.str());
        }

        // Traverse instructions from beginning to the end and find the first
        // instruction that is located after the end of the specification-block.
        // This assumes, that instructions are not reordered by the compiler.
        llvm::Instruction *closestInst = nullptr;
        for (auto it = inst_begin(parentFunc), end = inst_end(parentFunc); it != end; ++it)
        {
            // Get location of instruction
            auto dbgLoc = it->getDebugLoc();
            if (!dbgLoc)
                continue;
            const auto iLine = dbgLoc.getLine();
            const auto iCol = dbgLoc.getCol();

            // Skip instructions that are above/ before the spec-block
            if (iLine < endLine || (iLine == endLine && iCol <= endCol))
                continue;
            // Skip debug-intrinsics
            if (llvm::isa<llvm::DbgVariableIntrinsic>(&*it))
                continue;

            closestInst = &*it;
            break;
        }

        return closestInst;
    }
    llvm::Instruction *IRBasedSourceSpecMatcher::matchStatementBlock(SrcRange loc)
    {
        auto closestInst = getClosestInstructionAfter(loc);

        if (closestInst == nullptr)
        {
            std::stringstream s;
            s << "Failed to match statement-block at " << loc.asShortString();
            throw utils::PallasException(s.str());
        }

        if (printMatchingInfo)
            printStmntBlockMatchingInfo(loc, closestInst);
        return closestInst;
    }

    llvm::CallInst *
    IRBasedSourceSpecMatcher::matchGhostArgBindingBlock(SrcRange loc)
    {
        // Get the function in which the block is located:
        auto pFunc = getFirstFunctionAbove(loc.getStartLine());
        if (pFunc == nullptr)
            throw utils::PallasException(
                "Failed to get enclosing function for ghost argument binding.");

        // Find the closest call-instruction in front of the spec.
        llvm::CallInst *closestCall = nullptr;
        for (auto it = inst_begin(pFunc), e = inst_end(pFunc); it != e; ++it)
        {
            auto call = llvm::dyn_cast<llvm::CallInst>(&*it);
            if (call == nullptr)
                continue;
            // Get location of instruction
            auto dbgLoc = call->getDebugLoc();
            if (!dbgLoc)
                continue;
            const auto iLine = dbgLoc.getLine();
            const auto iCol = dbgLoc.getCol();

            // Skip calls after the start of the spec:
            if (iLine > loc.getStartLine() ||
                (iLine == loc.getStartLine() && iCol > loc.getStartCol()))
                continue;

            // Skip debug-intrinsics
            if (llvm::isa<llvm::DbgVariableIntrinsic>(&*it))
                continue;

            // Check if current call is closer than current closest call
            if (closestCall == nullptr)
                closestCall = call;
            else
            {
                auto closestLine = closestCall->getDebugLoc().getCol();
                auto closestCol = closestCall->getDebugLoc().getLine();
                if (iLine > closestLine ||
                    (iLine == closestLine && iCol > closestCol))
                {
                    closestCall = call;
                }
            }
        }

        if (closestCall == nullptr)
            throw utils::PallasException(
                "Failed to match ghost argument binding");

        if (printMatchingInfo)
            printGhostArgBindingMatchingInfo(loc, closestCall);

        return closestCall;
    }

    llvm::Function *IRBasedSourceSpecMatcher::getFirstFunctionAbove(
        const size_t line)
    {
        // Find the first function above the given line
        llvm::Function *parentFunction = nullptr;
        unsigned int parentStartLine = 0;
        for (auto &f : *irModule)
        {
            if (auto subprog = utils::getDeclSubprog(f))
            {
                // Skip functions that are not in the main-file.
                if (!isFromMainFile(*subprog))
                    continue;
                // TODO: Determine the end of the scope by iterating over all
                // instruction in the subscopes and comparing the line-numbers.
                auto fLine = subprog->getLine();
                // Skip all functions that start after the spec-block
                if (fLine > line)
                    continue;
                if (parentFunction == nullptr || fLine > parentStartLine)
                {
                    // The current funct is below the current parent funct.
                    parentFunction = &f;
                    parentStartLine = fLine;
                }
            }
        }
        return parentFunction;
    }

    bool IRBasedSourceSpecMatcher::isFromMainFile(
        const llvm::DISubprogram &sProg)
    {
        return utils::isFromFile(sProg, file);
    }
}