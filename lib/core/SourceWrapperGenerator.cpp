#include "SourceWrapperGenerator.h"
#include "PallasException.h"
#include "FileUtils.h"
#include "IRUtils.h"
#include "MetadataUtils.h"
#include "SrcLookup.h"

#include <algorithm>
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
        const auto pFunc = contract.function;

        // Get Wrapper-signature
        std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>> argList;
        std::optional<WrapperSignature> sig = std::nullopt;
        if (contract.type == External && contract.wrapperSignature.has_value())
        {
            // Get signature that was explicitly defined in contract
            sig = contract.wrapperSignature.value();
            // TODO: Handle encoding of args in this case.
            argList = std::make_unique<llvm::SetVector<llvm::DILocalVariable *>>();
        }
        else if (pFunc->getSubprogram() != nullptr &&
                 pFunc->getSubprogram()->getType() != nullptr)
        {
            // Construct signature from debug-info
            argList = getSrcVariablesForFunction(*pFunc);
            auto rType = pFunc->getSubprogram()->getType()->getTypeArray()[0];
            sig = getWrapperSignature(argList->getArrayRef(), rType);
        }

        if (!sig.has_value())
            throw utils::PallasException("Unable to get signature for wrapper");

        auto insertPos = (contract.type == Normal)
                             ? getWrapperInsertLoc(contract)
                             : getExternalWrapperInsertLoc(contract);

        // Wrap clauses of the contract
        for (auto &stmnt : contract.statements)
        {
            auto wName = getNameForWrapperFunction();
            auto wStr = getWrapperFunctionString(sig.value(), stmnt->expr, wName);
            matchedWrappers.fContractWrappers[stmnt.get()] =
                std::make_unique<SrcWrapperFunction>(
                    wName, wStr, argList->getArrayRef(), insertPos);
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
            if (child == nullptr)
                return false;
            if (child == parent)
                return true;
            // We first walk up the scope chain. Once we reach the end without
            // finding a parent, we resort to the file-field of the child.
            // E.g. DICompositeTypes often have no parent-scope, but a parent-file.
            if (child->getScope() != nullptr)
                return isParentScope(parent, child->getScope());
            else
                return isParentScope(parent, child->getFile());
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
                if (currentParent->getScope() != nullptr)
                    currentParent = currentParent->getScope();
                else
                    currentParent = currentParent->getFile();
            }
            return nullptr;
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
            auto wSig = getWrapperSignature(argList->getArrayRef());
            auto wrapperStr = getWrapperFunctionString(
                wSig, inv->expr, wrapperName);
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
            auto wSig = getWrapperSignature(argList->getArrayRef());
            auto wrapperStr = getWrapperFunctionString(
                wSig, stmnt->expr, wrapperName);
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
        const llvm::DominatorTree domTree(*parentFunc);

        // Get the scope of the loop header:
        auto *loopScope = utils::getScopeOfBasicBlock(*loopHeader);
        if (loopScope == nullptr)
            throw utils::PallasException(
                "Cannot determine scope for loop header");

        // Get all llvm.dbg.-intrinsics that dominate the loop and that 
        // are in scope accrding to the debug-scopes.
        for (auto i = llvm::inst_begin(parentFunc),
                  end = llvm::inst_end(parentFunc);
             i != end; ++i)
        {
            if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*i))
            {
                auto *srcVar = intr->getVariable();
                if (srcVar == nullptr ||
                    !domTree.dominates(intr, loopHeader) ||
                    !utils::isSubscope(srcVar->getScope(), loopScope))
                    continue;
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
        auto *instScope = utils::getScopeOfInstruction(*inst);
        // TODO: Print the instruction as well
        if (instScope == nullptr) 
            throw utils::PallasException(
                "Cannot determine scope of instruction.");

        // Some compilers place the debug-intrinsics for the function-arguments
        // later in the code, we add them explicitly.
        // TODO: This might break wrapper-generation, if there is shadowing!
        auto varSet = getSrcVariablesForFunction(*parentFunc);

        // Collect all variables from the llvm.dbg.-intrinsics that
        // dominate the instruction and that are in scope 
        // (according to the debug-scopes.)
        const llvm::DominatorTree domTree(*parentFunc);
        for (auto i = llvm::inst_begin(parentFunc),
                  end = llvm::inst_end(parentFunc);
             i != end; ++i)
        {
            if (&*i == inst)
                continue;

            if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*i))
            {
                auto *srcVar = intr->getVariable();
                // The instruction is a debug-intrinsic
                if (srcVar == nullptr ||
                    !domTree.dominates(&*i, inst) ||
                    !utils::isSubscope(srcVar->getScope(), instScope))
                    continue;
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

    std::string SourceWrapperGenerator::buildArgStr(const std::string &type,
                                                    const std::string &name)
    {
        return type + " " + name;
    }

    WrapperSignature SourceWrapperGenerator::getWrapperSignature(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const llvm::DIType *returnType)
    {
        WrapperSignature sig;
        for (auto *var : srcVars)
        {
            const auto varName = utils::getSrcVarName(*var);
            const auto varType = getSrcReprOfType(var->getType());
            const auto varStr = buildArgStr(varType, varName);
            sig.addArg(varStr);
        }

        if (returnType != nullptr)
        {
            sig.parentRetTy = getSrcReprOfType(returnType);
        }

        return sig;
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
        return utils::getDeclSubprog(*contract.function)->getLine();
    }

    size_t SourceWrapperGenerator::getWrapperInsertLoc(
        const MatchedLoopInvariantBlock &invBlock)
    {
        return utils::getDeclSubprog(*invBlock.parentFunction)->getLine();
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
        return utils::getDeclSubprog(*parentFunc)->getLine();
    }

    namespace
    {
        llvm::SmallVector<llvm::DIScope *> getCallsiteScopes(
            const llvm::Function &func)
        {
            llvm::SmallVector<llvm::DIScope *> scopes;
            for (auto *user : func.users())
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
            return scopes;
        }

    }

    size_t SourceWrapperGenerator::getExternalWrapperInsertLoc(
        const MatchedFunctionContract &contract)
    {
        auto contrFile = utils::makeAbsPath(contract.srcLoc.getFile());
        auto absSrcFile = utils::makeAbsPath(srcInFile);
        // Contract in src-file --> Place at location of contract
        if (contrFile == absSrcFile)
            return contract.srcLoc.getStartLine();

        auto *pFunc = contract.function;
        // Contract in external file --> Location based on marker-annotations
        llvm::DIScope *insertScope = nullptr;
        if (contract.type == Generated)
        {
            // Insert into scope of parent-func
            auto sProg = utils::getDeclSubprog(*pFunc);
            insertScope = sProg != nullptr
                              ? sProg->getScope()
                              : nullptr;
        }
        else if (contract.type == External)
        {
            // Collect scopes of all call-sites
            auto scopes = getCallsiteScopes(*pFunc);
            // Find the closest parent-scope of all call sites
            insertScope = getClosestParentScope(scopes);
        }

        if (insertScope == nullptr)
            throw utils::PallasException(
                "Wrapper generation: Failed to get insert-scope for contract");

        // Find matching insertion-point marker
        auto *insertMarker = selectLocMarker(*insertScope, wrapperMarkers);
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
        std::list<SrcLine> &srcLines)
    {
        // Do nothing. Subclasses can overwrite this
    }

    namespace
    {
        inline bool isInsertLine(const std::string &line)
        {
            return std::all_of(line.begin(), line.end(), isspace);
        }
    }

    std::list<SourceWrapperGenerator::SrcLine>::iterator
    SourceWrapperGenerator::findInsertPos(
        const size_t srcLineNumber, std::list<SrcLine> &srcLines)
    {
        // Find the index of the line of the insert-position in the vector
        auto currentLine = srcLines.begin();
        while (currentLine != srcLines.end() && (currentLine->origLine == 0 ||
                                                 currentLine->origLine < srcLineNumber))
            currentLine++;
        // Move back in the vector to find the first blank line above
        // the insert-position
        while (currentLine != srcLines.begin() &&
               !isInsertLine(currentLine->content))
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

    std::unique_ptr<std::list<SourceWrapperGenerator::SrcLine>>
    SourceWrapperGenerator::readSrcFile(std::string filePath)
    {
        std::ifstream inStream(filePath);
        if (!inStream.is_open())
            throw utils::PallasException("Failed to read source-file during wrapping-stage");

        auto lineVec = std::make_unique<std::list<SrcLine>>();
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
                                              const std::list<SrcLine> &lines)
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
        std::list<SrcLine> &lines, const std::string &content,
        std::list<SrcLine>::iterator insertLine)
    {
        // Construct vector that contains the lines of the content
        std::stringstream contentStream(content);
        std::string currentCLine;
        std::list<SrcLine> contentList;
        contentList.push_back(SrcLine("", 0));
        while (std::getline(contentStream, currentCLine))
            contentList.push_back(SrcLine(currentCLine, 0));
        // Insert content of vector
        lines.insert(insertLine, contentList.begin(), contentList.end());
    }
}