
#include "IRBasedSourceSpecMatcher.h"
#include "PallasException.h"
#include "SrcLookup.h"

#include <stdexcept>
#include <string>
#include <utility>

#include <llvm/ADT/SmallSet.h>
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
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
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
                              size_t loopSLine)
    {
        auto specLocStr = getLocationString(specSLine, specELine);
        auto loopLocStr = std::to_string(loopSLine);
        llvm::outs() << "Matched invariant at " << specLocStr
                     << " to loop at " << loopLocStr << "\n";
    }

    void printStmntBlockMatchingInfo(size_t specSLine, size_t specELine,
                                     size_t instLine, size_t instCol)
    {
        auto specLocStr = getLocationString(specSLine, specELine);
        auto instLocStr = std::to_string(instLine) + "::" +
                          std::to_string(instCol);
        llvm::outs() << "Matched statement-block at " << specLocStr
                     << " to instruction at " << instLocStr << "\n";
    }

    /**
     * Returns a tuple with the minimum and maximum line-numbers
     * that are covered by the given loop.
     */
    std::pair<unsigned int, unsigned int> getLines(const llvm::Loop &loop)
    {

        auto &startLoc = loop.getLocRange().getStart();
        auto &endLoc = loop.getLocRange().getEnd();

        if (!startLoc || !endLoc)
            throw sourcespec2ir::utils::PallasException(
                "Failed to get line-info for invariant");

        auto minLine = startLoc.getLine();
        auto maxLine = endLoc.getLine();
        return std::make_pair(minLine, maxLine);
    }

    /**
     * Uses llvm::sys::fs::make_absolute to turn the given path into an absolute
     * path.
     *
     * TODO: Move into the utils ?!
     */
    std::string makeAbsPath(std::string path)
    {
        if (llvm::sys::path::is_absolute(path))
            return path;

        llvm::SmallString<256> pathVec = llvm::StringRef(path);
        llvm::sys::fs::make_absolute(pathVec);
        auto absPath = llvm::sys::path::remove_leading_dotslash(pathVec);
        return absPath.str();
    }

}

namespace sourcespec2ir
{

    IRBasedSourceSpecMatcher::IRBasedSourceSpecMatcher(
        SpecParser::FileContext *specParseTree, llvm::Module *irModule,
        const std::string &srcFilePath,
        llvm::FunctionAnalysisManager *fam, const bool printMatchingInfo)
        : SourceSpecMatcher(specParseTree, irModule),
          srcFilePath(srcFilePath),
          printMatchingInfo(printMatchingInfo), fam(fam)
    {
    }

    std::string IRBasedSourceSpecMatcher::matchFContract(SrcRange loc)
    {
        const llvm::Function *closestFunction = nullptr;
        auto endLine = loc.getEnd()->getLine();

        // Set for checking that all functions are declared on distinct lines.
        llvm::SmallSet<unsigned int, 16> lineNumberSet;

        // Get first function under the contract
        for (auto &f : irModule->getFunctionList())
        {
            // Get location-information
            auto diSubprog = f.getSubprogram();
            if (diSubprog != nullptr)
            {
                if (!isFromMainFile(*diSubprog))
                    continue;

                // If the function is an artificially generated one, skip it.
                // This is e.g. the case for the default-arguments in swift. 
                if (isArtificial(diSubprog))
                    continue;

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
                if (line > endLine &&
                    (closestFunction == nullptr || line < closestFunction->getSubprogram()->getLine()))
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
                if (!f.isIntrinsic() && printMatchingInfo)
                {
                    auto fName = f.getName().str();
                    llvm::outs()
                        << "WARNING: Unable to determine line for function "
                        << fName << "\n";
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

        // Check that no instructions are between the specification and the function declaration.
        // TODO: Is this strictly nessessary? Based on the above algorithm, there should never be an
        //       instruction between contract and matched function.
        auto fLine = closestFunction->getSubprogram()->getLine();
        for (auto &f : irModule->getFunctionList())
        {
            // Skip artificial functions
            auto sProg = f.getSubprogram();
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
                if (iLine >= endLine && iLine < fLine)
                {
                    throw utils::PallasException(
                        "Encountered an instruction between contract and function.");
                }
                else if (iLine >= endLine && iLine == fLine && !isDebugIntrinsic)
                {
                    throw utils::PallasException(
                        "Encountered an instruction on the same line as the function.");
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
            printContractMatchingInfo(
                loc.getStartLine(), loc.getEndLine(),
                utils::getSrcNameOfFunction(*closestFunction));
        }

        return utils::getSrcNameOfFunction(*closestFunction);
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
            printInvMatchingInfo(startLine, loc.getEndLine(), closestLoopStart);

        return std::make_pair(parentFunction, closestLoop);
    }

    llvm::Instruction *IRBasedSourceSpecMatcher::matchStatementBlock(SrcRange loc)
    {
        // Get the first function above the spec-block
        // TODO: Consider the range that is covered by the function instead of
        // just considering the starting-line.
        auto startLine = loc.getStartLine();
        auto endLine = loc.getEndLine();
        auto endCol = loc.getEndCol();
        auto parentFunc = getFirstFunctionAbove(startLine);

        if (parentFunc == nullptr)
            throw utils::PallasException(
                "Failed to get enclosing function for statement-block");

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

        if (closestInst == nullptr)
            throw utils::PallasException("Failed to match statement-block");

        if (printMatchingInfo)
        {
            printStmntBlockMatchingInfo(startLine, endLine,
                                        closestInst->getDebugLoc().getLine(),
                                        closestInst->getDebugLoc().getCol());
        }

        return closestInst;
    }

    llvm::Function *IRBasedSourceSpecMatcher::getFirstFunctionAbove(
        const size_t line)
    {
        // Find the first function above the given line
        llvm::Function *parentFunction = nullptr;
        unsigned int parentStartLine = 0;
        for (auto &f : *irModule)
        {
            if (auto subprog = f.getSubprogram())
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
        // Check that the file that is referenced from the DISubprogram
        // of the given function matches the source-file from which the
        // specifications were extracted.
        const auto specFilePath = makeAbsPath(srcFilePath);

        // Build a scope for the source-file from which the function originates.
        // Some compilers put an absolute-path into the filename-field, while
        // others split the path between the filename- and the directory-field.
        auto fPath = sProg.getFilename().str();
        if (!llvm::sys::path::is_absolute(fPath))
        {
            // Build the path for the scope of the function by joining directory
            // and filename. Sometimes, a leading ./ occurs, which we remove.
            const auto fFile = llvm::sys::path::remove_leading_dotslash(
                sProg.getFilename());
            const auto fDir = llvm::sys::path::remove_leading_dotslash(
                sProg.getDirectory());
            const auto fFullPath = fDir +
                                   llvm::sys::path::get_separator() + fFile;
            fPath = fFullPath.str();
        }

        // llvm::errs() << "=================================\n";
        // llvm::errs() << "Function: " << sProg.getName().str() << "\n";
        // llvm::errs() << "Spec-file:   " << specFilePath << "\n";
        // llvm::errs() << "Function-file: " << fPath << "\n";

        return fPath == specFilePath;
    }
}