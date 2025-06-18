#include "SourceWrapperGenerator.h"
#include "PallasException.h"
#include "FileUtils.h"

#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <vector>
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/Support/FileSystem.h>

namespace sourcespec2ir
{

    SourceWrapperGenerator::SourceWrapperGenerator(
        const std::string srcInFile,
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations,
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers)
        : srcInFile(srcInFile),
          specDeclarations(specDeclarations),
          wrapperMarkers(wrapperMarkers)
    {
    }

    std::unique_ptr<MatchedWrapperFunctions>
    SourceWrapperGenerator::generateWrapperFunctions(
        const MatchedSpecifications &matchedSpecs)
    {
        auto matchedWrappers = std::make_unique<MatchedWrapperFunctions>();

        // Create wrappers for function-contracts
        for (const auto &contract : matchedSpecs.matchedFContracts)
            wrapFunctionContractBlock(*contract, *matchedWrappers);
        // Create wrappers for external contracts
        for (const auto &contract : matchedSpecs.matchedExternalContracts)
            wrapExternalContractBlock(*contract, *matchedWrappers);
        // Create wrappers for loop-invariants
        for (const auto &invBlock : matchedSpecs.matchedLoopInvariants)
            wrapLoopInvariantBlock(*invBlock, *matchedWrappers);
        // Create wrappers for blocks of specification-statements
        for (const auto &stmntBlock : matchedSpecs.matchedStmntBlocks)
            wrapSpecStatementBlock(*stmntBlock, *matchedWrappers);

        return matchedWrappers;
    }

    void SourceWrapperGenerator::wrapFunctionContractBlock(
        const MatchedFunctionContract &contract,
        MatchedWrapperFunctions &matchedWrappers)
    {
        const auto parentFunction = contract.function;
        llvm::DIType *parentRetType = nullptr;
        if (parentFunction->getSubprogram() != nullptr &&
            parentFunction->getSubprogram()->getType() != nullptr)
        {
            parentRetType = parentFunction->getSubprogram()->getType()->getTypeArray()[0];
        }

        auto argList = getSrcVariablesForFunction(*parentFunction);
        // Get insert-position for wrapper-functions
        auto insertPos = getWrapperInsertLoc(contract);

        // Wrap specification statements
        for (auto &stmnt : contract.statements)
        {
            auto wrapperName = getNameForWrapperFunction();
            auto wrapperStr = getWrapperFunctionString(
                argList->getArrayRef(), stmnt->expr, wrapperName, parentRetType);
            matchedWrappers.fContractWrappers[stmnt.get()] =
                std::make_unique<SrcWrapperFunction>(
                    wrapperName, wrapperStr, argList->getArrayRef(), insertPos);
        }
    };

    namespace
    {

        const sourcespec2ir::WrapperLocMarker *selectLocMarker(
            const llvm::DIScope &scope,
            const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers)
        {
            // Filter for loc-markers that match the name of the scope
            llvm::SmallVector<const sourcespec2ir::WrapperLocMarker *> matchingMarkers;
            llvm::SmallVector<const sourcespec2ir::WrapperLocMarker *> wildcardMarkers;
            for (auto &marker : wrapperMarkers)
            {
                if (marker.scopeMatchingStr.has_value())
                {
                    const std::regex matchRegex(*marker.scopeMatchingStr);
                    if (std::regex_match(scope.getName().str(), matchRegex))
                        matchingMarkers.push_back(&marker);
                }
                else
                {
                    // Treat markers without matching-string as wildcards
                    wildcardMarkers.push_back(&marker);
                }
            }

            if (matchingMarkers.size() == 1)
                return matchingMarkers[0];
            if (matchingMarkers.size() == 0 && wildcardMarkers.size() == 1)
                return wildcardMarkers[0];
            if (matchingMarkers.empty() && wildcardMarkers.empty())
                return nullptr;

            // TODO: How to handle multiple matching markers?
            throw utils::PallasException(
                "Wrapper generation: Ambiguous insert-location "
                "for external contract ");
        }

        bool isParentScope(const llvm::DIScope *parent,
                           const llvm::DIScope *child)
        {
            if (child == parent)
                return true;
            return isParentScope(parent, child->getScope());
        }

        llvm::DIScope *getClosestParentScope(
            llvm::ArrayRef<llvm::DIScope *> children)
        {
            // TODO: Implement this in a less inefficient way!
            if (children.empty())
                return nullptr;
            auto *currentParent = children[0]->getScope();
            // Move up to the next parent-scope until we fine one that includes
            // all child-scopes
            while (currentParent != nullptr)
            {
                bool isParent = true;
                for (auto *c : children)
                {
                    if (!isParentScope(currentParent, c))
                    {
                        isParent = false;
                        break;
                    }
                }
                if (isParent)
                    return currentParent;
                currentParent = currentParent->getScope();
            }
            return nullptr;
        }
    }

    void SourceWrapperGenerator::wrapExternalContractBlock(
        const MatchedFunctionContract &contract,
        MatchedWrapperFunctions &matchedWrappers)
    {
        // TODO: Handle the case when there is no debug-info
        if (contract.function->getSubprogram() == nullptr ||
            contract.function->getSubprogram()->getType() == nullptr)
            throw utils::PallasException(
                "Unable to get wrapper-arguments (missing dbg-info)");
        auto argList = getSrcVariablesForFunction(*contract.function);
        auto rType = contract.function->getSubprogram()->getType()->getTypeArray()[0];
        auto loc = getExternalWrapperInsertLoc(contract);

        // Wrap clauses of the contract
        for (auto &stmnt : contract.statements)
        {
            auto wName = getNameForWrapperFunction();
            auto wStr = getWrapperFunctionString(
                argList->getArrayRef(), stmnt->expr, wName, rType);
            matchedWrappers.fContractWrappers[stmnt.get()] =
                std::make_unique<SrcWrapperFunction>(
                    wName, wStr, argList->getArrayRef(), loc);
        }
    }

    void SourceWrapperGenerator::wrapLoopInvariantBlock(
        const MatchedLoopInvariantBlock &invBlock,
        MatchedWrapperFunctions &matchedWrappers)
    {
        // Determine the variables that can be referenced in the invariant
        auto argList = getVarsForLoopInv(
            invBlock.parentFunction, invBlock.loopHeader);
        auto insertPos = getWrapperInsertLoc(invBlock);

        // Generate wrappers for the individual invariants
        for (auto &inv : invBlock.invariants)
        {
            auto wrapperName = getNameForWrapperFunction();
            auto wrapperStr = getWrapperFunctionString(
                argList->getArrayRef(), inv->expr, wrapperName);
            matchedWrappers.loopInvWrappers[inv.get()] =
                std::make_unique<SrcWrapperFunction>(
                    wrapperName, wrapperStr, argList->getArrayRef(), insertPos);
        }
    }

    void SourceWrapperGenerator::wrapSpecStatementBlock(
        const MatchedSpecStatementBlock &stmntBlock,
        MatchedWrapperFunctions &matchedWrappers)
    {
        // Determine the variables that can be referenced in the statement
        auto argList = getVarsForStatement(stmntBlock.inst);
        // Get insert-location for wrapper-function
        auto insertPos = getWrapperInsertLoc(stmntBlock);

        // Generate wrapper for the individual statements
        for (auto &stmnt : stmntBlock.statements)
        {
            auto wrapperName = getNameForWrapperFunction();
            auto wrapperStr = getWrapperFunctionString(
                argList->getArrayRef(), stmnt->expr, wrapperName);
            matchedWrappers.statementWrappers[stmnt.get()] =
                std::make_unique<SrcWrapperFunction>(
                    wrapperName, wrapperStr, argList->getArrayRef(), insertPos);
        }
    }

    std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>>
    SourceWrapperGenerator::getSrcVariablesForFunction(const llvm::Function &fun)
    {
        if (fun.isDeclaration())
            throw utils::PallasException("Cannot determine variables for declaration.");

        // SetVector is used to keep insertion-order and prevent duplicates
        auto varSet = std::make_unique<llvm::SetVector<llvm::DILocalVariable *>>();
        // Iterate over all of the llvm.dbg.-intrinsics and get the names and types of the variables
        for (auto i = llvm::inst_begin(fun), end = llvm::inst_end(fun); i != end; ++i)
        {
            const llvm::Instruction *instr = &*i;
            // Only consider calls to llvm intrinsics
            if (auto *intrinsic = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(instr))
            {
                auto *srcVar = intrinsic->getVariable();
                // Filter on function arguments
                if (srcVar->getArg() != 0)
                    varSet->insert(srcVar);
            }
        }

        return varSet;
    }

    std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>>
    SourceWrapperGenerator::getVarsForLoopInv(llvm::Function *parentFunc,
                                              const llvm::BasicBlock *loopHeader)
    {
        // Since some compilers place the debug-intrinsics for the function-arguments
        // later in the code, we add them explicitly.
        // TODO: This might break wrapper-generation, if there is shadowing!
        auto varSet = getSrcVariablesForFunction(*parentFunc);
        // auto varSet = std::make_unique<llvm::SetVector<llvm::DILocalVariable *>>();
        const llvm::DominatorTree domTree(*parentFunc);

        // Get all llvm.dbg.-intrinsics that dominate the loop
        for (auto i = llvm::inst_begin(parentFunc),
                  end = llvm::inst_end(parentFunc);
             i != end; ++i)
        {
            if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*i))
            {
                if (!domTree.dominates(intr, loopHeader))
                    continue;
                auto *srcVar = intr->getVariable();
                varSet->insert(srcVar);
            }
        }

        return varSet;
    }

    std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>>
    SourceWrapperGenerator::getVarsForStatement(llvm::Instruction *inst)
    {
        // Get the parent-function of the instruction
        if (inst == nullptr ||
            inst->getParent() == nullptr ||
            inst->getParent()->getParent() == nullptr)
        {
            throw utils::PallasException(
                "Encountered instruction without parent");
        }
        auto parentFunc = inst->getParent()->getParent();

        // Some compilers place the debug-intrinsics for the function-arguments
        // later in the code, we add them explicitly.
        // TODO: This might break wrapper-generation, if there is shadowing!
        auto varSet = getSrcVariablesForFunction(*parentFunc);

        // Collect all variables from the llvm.dbg.-intrinsics that
        // dominate the instruction
        // auto varSet = std::make_unique<llvm::SetVector<llvm::DILocalVariable *>>();
        const llvm::DominatorTree domTree(*parentFunc);
        for (auto i = llvm::inst_begin(parentFunc),
                  end = llvm::inst_end(parentFunc);
             i != end; ++i)
        {
            if (&*i == inst)
                continue;

            if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*i))
            {
                // The instruction is a debug-intrinsic
                if (!domTree.dominates(&*i, inst))
                    continue;
                auto *srcVar = intr->getVariable();
                varSet->insert(srcVar);
            }
        }
        return varSet;
    }

    std::string SourceWrapperGenerator::getSrcReprOfType(const llvm::DIType *type)
    {
        // Check if type is a derived type
        if (auto *dType = llvm::dyn_cast<llvm::DIDerivedType>(type))
        {
            const auto tag = dType->getTag();
            const auto baseType = dType->getBaseType();
            const auto baseTyStr = getSrcReprOfType(baseType);

            if (tag == llvm::dwarf::DW_TAG_pointer_type)
                return addPointer(baseTyStr);
            else if (tag == llvm::dwarf::DW_TAG_const_type)
                return addConst(baseTyStr);
            else if (tag == llvm::dwarf::DW_TAG_reference_type)
                return addReference(baseTyStr);
            else if (tag == llvm::dwarf::DW_TAG_typedef)
                return addTypedef(baseTyStr, dType->getName().str());
            else
                return addOtherQualifier(baseTyStr, *dType);
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

    std::string SourceWrapperGenerator::addConst(const std::string &baseType)
    {
        // Override this to support const.
        throw utils::PallasException(
            "Const is not supported by this WrapperGenerator.");
    }

    std::string SourceWrapperGenerator::addPointer(const std::string &baseType)
    {
        // Override this to support pointers.
        throw utils::PallasException(
            "Pointers are not supported by this WrapperGenerator.");
    }

    std::string SourceWrapperGenerator::addReference(const std::string &baseType)
    {
        // Override this to support references.
        throw utils::PallasException(
            "References are not supported by this WrapperGenerator.");
    }

    std::string SourceWrapperGenerator::addTypedef(
        const std::string &baseType, const std::string &typedefName)
    {
        // Override this to support typedefs.
        throw utils::PallasException(
            "Typedefs are not supported by this WrapperGenerator.");
    }

    std::string SourceWrapperGenerator::addOtherQualifier(
        const std::string &baseType, const llvm::DIDerivedType &type)
    {
        // Override this to support other type-tags that are not
        // supported by the default-implementation.
        throw utils::PallasException("Encountered an unsupported type-tag");
    }

    std::string SourceWrapperGenerator::addNamespace(
        const std::string &parentNamespace, const std::string &childNamespace)
    {
        // Override this to support namespaces.
        throw utils::PallasException(
            "Namespaces are not supported by this WrapperGenerator.");
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

    size_t SourceWrapperGenerator::getWrapperInsertLoc(
        const MatchedSpecStatementBlock &stmntBlock)
    {
        auto instr = stmntBlock.inst;
        // Get the parent-function of the instruction
        if (instr == nullptr ||
            instr->getParent() == nullptr ||
            instr->getParent()->getParent() == nullptr)
        {
            throw utils::PallasException(
                "Cannot get parent-function for instruction");
        }
        auto parentFunc = instr->getParent()->getParent();
        if (parentFunc->getSubprogram() == nullptr)
            throw utils::PallasException("Cannot get line-info for function");

        return parentFunc->getSubprogram()->getLine();
    }

    size_t SourceWrapperGenerator::getExternalWrapperInsertLoc(
        const MatchedFunctionContract &contract)
    {
        auto *pFunc = contract.function;

        // Collect scopes of all call-sites
        llvm::SmallVector<llvm::DIScope *> scopes;
        for (auto *user : pFunc->users())
        {
            if (auto *call = llvm::dyn_cast<llvm::CallBase>(user))
            {
                auto *scope = call->getFunction()->getSubprogram();
                if (scope == nullptr)
                    throw utils::PallasException(
                        "Wrapper generation: Failed to get scope from call-site");
                scopes.push_back(scope);
            }
        }
        // Find the closest parent-scope of all call sites
        auto *parentScope = getClosestParentScope(scopes);
        if (parentScope == nullptr)
            throw utils::PallasException(
                "Wrapper generation: Failed to get parent-scope for external contract");
        // Find matching insertion-point marker
        auto *insertMarker = selectLocMarker(*parentScope, wrapperMarkers);
        if (insertMarker == nullptr)
            throw utils::PallasException(
                "Wrapper generation: Failed to get insert location for external contract");
        return insertMarker->markerLoc.getStartLine();
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
        llvm::ArrayRef<SrcWrapperFunction *> wrapperFunctions)
    {
        // Load file as vector of lines
        auto sourceLines = readSrcFile(inPath);

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
        // If the file already exists, delete it to avoid a situation where
        // writing fails, but an old, existing file is read.
        auto rmSucc = llvm::sys::fs::remove(filePath);
        if (rmSucc)
            throw utils::PallasException(
                "Failed to remove existing wrapper-file.");

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