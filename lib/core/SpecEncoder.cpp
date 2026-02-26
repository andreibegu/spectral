#include "SpecEncoder.h"

#include "FileUtils.h"
#include "PallasException.h"
#include "MetadataUtils.h"
#include "SrcLookup.h"

#include <sstream>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IntrinsicInst.h>

namespace sourcespec2ir
{

    SpecEncoder::SpecEncoder(
        llvm::LLVMContext &irContext, llvm::Module &irModule,
        const MatchedWrapperFunctions &matchedWrappers)
        : irContext(irContext), irModule(irModule),
          matchedWrappers(matchedWrappers), ghostArgDefMap() {}

    void SpecEncoder::encodeSpecifications(const MatchedSpecifications &specs)
    {
        // Encode contracts first, because they declare ghost variables
        for (const auto &contract : specs.matchedFContracts)
            encodeFunctionContractBlock(*contract);

        // Loop invariants
        for (const auto &invBlock : specs.matchedLoopInvariants)
        {
            auto *parentContr = specs.getContractForFunc(
                *invBlock->parentFunction);
            encodeLoopInvBlock(*invBlock, parentContr);
        }

        // Spec-Statements
        for (const auto &stmntBlock : specs.matchedStmntBlocks)
        {
            auto *parentFunc = stmntBlock->inst->getParent()->getParent();
            auto *parentContr = specs.getContractForFunc(*parentFunc);
            encodeStmntBlock(*stmntBlock, parentContr);
        }

        // Ghost contracts
        for (const auto &contract : specs.matchedGhostContracts)
            encodeGhostContract(*contract);

        // Given bindings
        for (const auto &bBlock : specs.matchedGivenBindings)
        {
            auto *calledFunc = bBlock->call->getCalledFunction();
            auto *calleeContr = specs.getContractForFunc(*calledFunc);
            auto *parentContr = specs.getContractForFunc(
                *bBlock->call->getParent()->getParent());
            if (calleeContr == nullptr)
                throwNoContractForFunc(*calledFunc);
            encodeGivenBindingBlock(*bBlock, *calleeContr, parentContr);
        }

        // Yields bindings
        for (const auto &bBlock : specs.matchedYieldsBindings)
        {
            auto *calledFunc = bBlock->call->getCalledFunction();
            auto *calleeContr = specs.getContractForFunc(*calledFunc);

            if (calleeContr == nullptr)
                throwNoContractForFunc(*calledFunc);
            auto *parentFunc = bBlock->call->getParent()->getParent();
            auto *parentContr = specs.getContractForFunc(*parentFunc);
            if (parentContr == nullptr)
                throwNoContractForFunc(*parentFunc);
            encodeYieldsBindingBlock(*bBlock, *parentContr, *calleeContr);
        }
    }

    void SpecEncoder::encodeFunctionContractBlock(
        const MatchedFunctionContract &contract)
    {
        const auto parentFunction = contract.function;
        llvm::SmallVector<llvm::Metadata *> contractMDNodes;

        // Add src-location information
        contractMDNodes.push_back(encodeSrcLoc(contract.srcLoc));

        // Add pure-flag
        auto pureFlagConst = llvm::ConstantAsMetadata::get(
            llvm::ConstantInt::getBool(irContext, contract.isPure));
        contractMDNodes.push_back(pureFlagConst);

        // Add assumed-flag
        auto assumedFlagConst = llvm::ConstantAsMetadata::get(
            llvm::ConstantInt::getBool(irContext, contract.isAssumed));
        contractMDNodes.push_back(assumedFlagConst);

        // Given args
        llvm::SmallVector<llvm::Metadata *> givenDefs;
        for (auto &g : contract.givenArgs)
            givenDefs.push_back(encodeGhostArgDef(g));
        auto givenNodes = llvm::MDTuple::get(irContext, givenDefs);
        contractMDNodes.push_back(givenNodes);

        // Yields args
        llvm::SmallVector<llvm::Metadata *> yieldsDefs;
        for (auto &y : contract.yieldsArgs)
            yieldsDefs.push_back(encodeGhostArgDef(y));
        auto yieldsNodes = llvm::MDTuple::get(irContext, yieldsDefs);
        contractMDNodes.push_back(yieldsNodes);

        // Transform specification statements
        for (auto &matchedStmnt : contract.statements)
        {
            auto mdNode = encodeFunctionContractStatement(*matchedStmnt,
                                                          &contract, false);
            contractMDNodes.push_back(mdNode);
        }

        auto contractMDNode = llvm::MDTuple::get(irContext, contractMDNodes);
        auto label = contract.type == External ? PALLAS_EXT_FUN_CONTRACT_BLOCK
                                               : PALLAS_FUN_CONTRACT_BLOCK;
        parentFunction->setMetadata(label, contractMDNode);
    }

    llvm::MDNode *SpecEncoder::encodeFunctionContractStatement(
        const MatchedFContractStatement &stmnt,
        const MatchedFunctionContract *parentContract,
        const bool inGhostContract)
    {
        if (!inGhostContract && parentContract == nullptr)
            throw utils::PallasException(
                "Unable to access definition of ghost variables!");

        // Get the ir-version of the wrapper-function
        auto &wrapperMap = inGhostContract
                               ? matchedWrappers.ghostContractWrappers
                               : matchedWrappers.fContractWrappers;
        auto srcWrapperFun = wrapperMap.at(&stmnt).get();
        auto irWrapperFunction = utils::getFunctionBySrcName(
            irModule, srcWrapperFun->name);

        // Add a metadata-marker to generated specification-function
        utils::markGeneratedSpecFunction(*irWrapperFunction, irContext);

        // Generate the metadata that encodes the contract statement:
        llvm::SmallVector<llvm::Metadata *> mdElems;

        // Statement identifier
        std::string mdId;
        if (stmnt.type == Requires)
            mdId = PALLAS_REQUIRES;
        else if (stmnt.type == Ensures)
            mdId = PALLAS_ENSURES;
        else
            throw utils::PallasException("Unsupported statement-type");
        auto mdIdentifier = llvm::MDString::get(irContext, mdId);
        mdElems.push_back(mdIdentifier);

        // Src-location information
        mdElems.push_back(encodeSrcLoc(stmnt.srcLoc));

        // Reference to wrapper function
        auto specFuncAsMd = llvm::ValueAsMetadata::get(irWrapperFunction);
        mdElems.push_back(specFuncAsMd);

        // Arg-mapping
        bool includeYields = stmnt.type != Requires;
        addWrapperArgMapping(
            mdElems, *srcWrapperFun, parentContract, includeYields);

        return llvm::MDNode::get(irContext, mdElems);
    }

    void SpecEncoder::encodeLoopInvBlock(
        const MatchedLoopInvariantBlock &invBlock,
        const MatchedFunctionContract *parentContract)
    {
        const auto numInvs = invBlock.invariants.size();
        if (numInvs == 0)
            return;

        llvm::SmallVector<llvm::Metadata *> invNodes;

        // ID-String
        auto invIDNode = llvm::MDString::get(irContext, PALLAS_LOOP_INV_BLOCK);
        invNodes.push_back(invIDNode);

        // Src location
        invNodes.push_back(encodeSrcLoc(invBlock.srcLoc));

        // Invariants
        for (auto &inv : invBlock.invariants)
            invNodes.push_back(encodeLoopInv(*inv, parentContract));

        // Construct & add MD-tuple
        auto loopMDTuple = invBlock.loopID;
        auto invNode = llvm::MDTuple::get(irContext, invNodes);
        loopMDTuple->push_back(invNode);
    }

    llvm::MDNode *SpecEncoder::encodeLoopInv(
        const MatchedLoopInvariant &inv,
        const MatchedFunctionContract *parentContract)
    {
        // IR-version of wrapper
        auto srcWrapperFun = matchedWrappers.loopInvWrappers.at(&inv).get();
        auto irWrapperFunction = utils::getFunctionBySrcName(
            irModule, srcWrapperFun->name);

        utils::markGeneratedSpecFunction(*irWrapperFunction, irContext);

        llvm::SmallVector<llvm::Metadata *> mdElems;

        // ID-String
        mdElems.push_back(llvm::MDString::get(irContext, PALLAS_LOOP_INV));

        // Src location
        mdElems.push_back(encodeSrcLoc(inv.srcLoc));

        // Wrapper Function
        mdElems.push_back(llvm::ValueAsMetadata::get(irWrapperFunction));

        // Arg-mapping
        addWrapperArgMapping(mdElems, *srcWrapperFun, parentContract);

        return llvm::MDNode::get(irContext, mdElems);
    }

    void SpecEncoder::encodeStmntBlock(
        const MatchedSpecStatementBlock &stmntBlock,
        const MatchedFunctionContract *parentContract)
    {
        const auto inst = stmntBlock.inst;
        if (stmntBlock.inst == nullptr)
            throw utils::PallasException("Encountered unmatched stmnt-block");

        llvm::SmallVector<llvm::Metadata *> nodes;

        // Location
        nodes.push_back(encodeSrcLoc(stmntBlock.srcLoc));

        // Transform specification statements
        for (auto &stmnt : stmntBlock.statements)
            nodes.push_back(encodeSpecStatement(*stmnt, parentContract));

        // Build md-node to attach to function
        auto blockMDNode = llvm::MDTuple::get(irContext, nodes);
        inst->setMetadata(PALLAS_STMNT_BLOCK, blockMDNode);
    }

    llvm::MDNode *SpecEncoder::encodeSpecStatement(
        const MatchedSpecStatement &stmnt,
        const MatchedFunctionContract *parentContract)
    {
        bool isGhostAssign = stmnt.type == GhostAssign;

        if (isGhostAssign && parentContract == nullptr)
            throw utils::PallasException(
                "Ghost assignments may only occur in functions that define "
                "ghost variables.");

        auto srcWrapperFun = matchedWrappers.statementWrappers.at(&stmnt).get();
        auto irWrapperFun = utils::getFunctionBySrcName(
            irModule, srcWrapperFun->name);

        // Mark wrapper function
        if (isGhostAssign)
            utils::markGhostWrapper(*irWrapperFun, irContext);
        else
            utils::markGeneratedSpecFunction(*irWrapperFun, irContext);

        // Generate the metadata that encodes the contract statement:
        llvm::SmallVector<llvm::Metadata *> mdElems;

        // Statement identifier
        if (!stmntTypeToID.contains(stmnt.type))
            throw utils::PallasException("Unsupported statement-type");
        mdElems.push_back(
            llvm::MDString::get(irContext, stmntTypeToID.at(stmnt.type)));

        // Add info on srcLocation
        mdElems.push_back(encodeSrcLoc(stmnt.srcLoc));

        // Add reference to function
        mdElems.push_back(llvm::ValueAsMetadata::get(irWrapperFun));

        // Arg-mapping
        addWrapperArgMapping(mdElems, *srcWrapperFun, parentContract);

        // MD-encoding of the variable definition (for ghost assign)
        if (isGhostAssign)
        {
            auto vDef = parentContract->getGhostArg(stmnt.ghostVarName.value());
            if (vDef == nullptr || !ghostArgDefMap.contains(vDef))
            {
                std::stringstream s;
                s << "Failed to find definition of ghost variable: "
                  << stmnt.ghostVarName.value() << " at "
                  << stmnt.srcLoc.asShortString();
                throw utils::PallasException(s.str());
            }
            auto *gVarMD = ghostArgDefMap.at(vDef);
            mdElems.push_back(gVarMD);
        }

        return llvm::MDNode::get(irContext, mdElems);
    }

    void SpecEncoder::encodeGhostContract(
        const MatchedGhostContract &contract)
    {
        auto pFunc = utils::getFunctionBySrcName(
            irModule, contract.method->name);
        if (pFunc == nullptr)
            throw utils::PallasException("Failed to get ghost-function for "
                                         "contract");

        llvm::SmallVector<llvm::Metadata *> nodes;
        // Src-location
        nodes.push_back(encodeSrcLoc(contract.srcLoc));
        // Pure-flag
        bool isFunction = contract.method->isFunction;
        nodes.push_back(llvm::ConstantAsMetadata::get(
            llvm::ConstantInt::getBool(irContext, isFunction)));
        // Assumed-flag
        // Always false for now, added to ensure consistency with encoding
        // of external function.
        nodes.push_back(llvm::ConstantAsMetadata::get(
            llvm::ConstantInt::getBool(irContext, false)));
        // Ghost contracts do not support defining given- / yields args
        // --> Add empty tuples to keep encoding compatible with regular contracts
        nodes.push_back(llvm::MDTuple::get(irContext, {}));
        nodes.push_back(llvm::MDTuple::get(irContext, {}));
        // Specification statements
        for (auto &matchedStmnt : contract.statements)
            nodes.push_back(encodeFunctionContractStatement(
                *matchedStmnt, nullptr, true));

        auto contractNode = llvm::MDTuple::get(irContext, nodes);
        pFunc->setMetadata(PALLAS_FUN_CONTRACT_BLOCK, contractNode);
    }

    void SpecEncoder::encodeGivenBindingBlock(
        const MatchedGhostArgBindingBlock &bBlock,
        const MatchedFunctionContract &calleeContract,
        const MatchedFunctionContract *parentContract)
    {
        llvm::SmallVector<llvm::Metadata *> nodes;

        // Src-location
        nodes.push_back(encodeSrcLoc(bBlock.srcLoc));

        // Bindings
        for (auto &b : bBlock.bindings)
            nodes.push_back(
                encodeGivenBinding(b, calleeContract, parentContract));

        // Attach node to function
        auto blockNode = llvm::MDTuple::get(irContext, nodes);
        bBlock.call->setMetadata(PALLAS_GIVEN_BINDING_BLOCK, blockNode);
    }

    llvm::MDNode *SpecEncoder::encodeGivenBinding(
        const MatchedGhostVarBinding &binding,
        const MatchedFunctionContract &calleeContract,
        const MatchedFunctionContract *parentContract)
    {
        auto srcWrapper = matchedWrappers.givenBindingWrappers.at(&binding).get();
        auto irWrapper = utils::getFunctionBySrcName(irModule, srcWrapper->name);

        // Add a metadata-marker to wrapper
        utils::markGhostWrapper(*irWrapper, irContext);

        llvm::SmallVector<llvm::Metadata *> nodes;

        // ID String
        nodes.push_back(llvm::MDString::get(irContext, PALLAS_GIVEN_BINDING));

        // Src location
        nodes.push_back(encodeSrcLoc(binding.srcLoc));

        // Wrapper function
        nodes.push_back(llvm::ValueAsMetadata::get(irWrapper));

        // Arg-mapping
        addWrapperArgMapping(nodes, *srcWrapper, parentContract);

        // MD-encoding of the arg-definition
        auto argDef = calleeContract.getGivenArg(binding.ghostArg);
        if (argDef == nullptr || !ghostArgDefMap.contains(argDef))
        {
            std::stringstream s;
            s << "Failed to find definition of given-argument: "
              << binding.ghostArg << " at " << binding.srcLoc.asShortString();
            throw utils::PallasException(s.str());
        }
        auto *argMD = ghostArgDefMap.at(argDef);
        nodes.push_back(argMD);

        return llvm::MDNode::get(irContext, nodes);
    }

    void SpecEncoder::encodeYieldsBindingBlock(
        const MatchedGhostArgBindingBlock &bBlock,
        const MatchedFunctionContract &parentContract,
        const MatchedFunctionContract &calleeContract)
    {
        llvm::SmallVector<llvm::Metadata *> nodes;

        // Src-location
        nodes.push_back(encodeSrcLoc(bBlock.srcLoc));

        // Bindings
        for (auto &b : bBlock.bindings)
            nodes.push_back(encodeYieldsBinding(
                b, parentContract, calleeContract));

        // Attach node to function
        auto blockNode = llvm::MDTuple::get(irContext, nodes);
        bBlock.call->setMetadata(PALLAS_YIELDS_BINDING_BLOCK, blockNode);
    }

    llvm::MDNode *SpecEncoder::encodeYieldsBinding(
        const MatchedGhostVarBinding &binding,
        const MatchedFunctionContract &parentContract,
        const MatchedFunctionContract &calleeContract)
    {
        llvm::SmallVector<llvm::Metadata *, 8> nodes;

        // ID String
        nodes.push_back(llvm::MDString::get(irContext, PALLAS_YIELDS_BINDING));

        // Src location
        nodes.push_back(encodeSrcLoc(binding.srcLoc));

        // Target var (ghost var from parent function)
        auto targetVarName = llvm::StringRef(binding.exprStr).trim().str();
        auto targetVarDef = parentContract.getGhostArg(targetVarName);
        if (targetVarDef == nullptr || !ghostArgDefMap.contains(targetVarDef))
        {
            std::stringstream s;
            s << "Failed to find definition of ghost-argument: "
              << binding.exprStr << " at " << binding.exprLoc.asShortString();
            throw utils::PallasException(s.str());
        }
        auto targetVarMD = ghostArgDefMap.at(targetVarDef);
        nodes.push_back(targetVarMD);

        // Source var (yields-arg from called function)
        auto sourceArgDef = calleeContract.getYieldsArg(binding.ghostArg);
        if (sourceArgDef == nullptr || !ghostArgDefMap.contains(sourceArgDef))
        {
            std::stringstream s;
            s << "Failed to find definition of yields-argument: "
              << binding.ghostArg << " at " << binding.srcLoc.asShortString();
            throw utils::PallasException(s.str());
        }
        auto *sourceArgMD = ghostArgDefMap.at(sourceArgDef);
        nodes.push_back(sourceArgMD);

        return llvm::MDNode::get(irContext, nodes);
    }

    llvm::MDTuple *SpecEncoder::encodeGhostArgDef(const GhostArgumentDef &argDef)
    {
        llvm::SmallVector<llvm::Metadata *> nodes;
        // location
        nodes.push_back(encodeSrcLoc(argDef.srcLoc));
        // name
        nodes.push_back(llvm::MDString::get(irContext, argDef.argName));

        auto mdEncoding = llvm::MDTuple::get(irContext, nodes);

        // Keep mapping of GhostArgumentDef to MD-nodes
        ghostArgDefMap[&argDef] = mdEncoding;
        return mdEncoding;
    }

    void SpecEncoder::addWrapperArgMapping(
        llvm::SmallVector<llvm::Metadata *> &nodes,
        const SrcWrapperFunction &srcWrapper,
        const MatchedFunctionContract *parentContr, bool includeYields)
    {
        auto irWrapper = utils::getFunctionBySrcName(irModule, srcWrapper.name);
        if (irWrapper == nullptr)
        {
            std::stringstream s;
            s << "Failed to find IR-function for wrapper with name "
              << srcWrapper.name;
            throw utils::PallasException(s.str());
        }

        // Given args
        nodes.push_back(
            encodeGhostVarSet(srcWrapper.givenArgs, *irWrapper, parentContr));

        // Yields args
        llvm::Metadata *yieldsNode = nullptr;
        if (includeYields)
            yieldsNode = encodeGhostVarSet(srcWrapper.yieldsArgs, *irWrapper,
                                           parentContr);
        else
            yieldsNode = llvm::MDTuple::get(irContext, {});
        nodes.push_back(yieldsNode);
        // 'Regular' variables
        nodes.push_back(encodeWrapperVarMapping(srcWrapper, *irWrapper));
    }

    llvm::MDNode *SpecEncoder::encodeWrapperVarMapping(
        const SrcWrapperFunction &srcWrapper,
        const llvm::Function &irWrapper) const
    {
        auto nameToWArg = mapDIArgsToNames(irWrapper);
        llvm::SmallVector<llvm::Metadata *> argMappingNodes;
        for (auto *pVar : srcWrapper.argList)
        {
            auto argName = pVar->getName().str();
            // Find the dbgvar in wrapper that belongs to 'pVar'
            if (!nameToWArg.contains(argName))
            {
                std::stringstream s;
                s << "Failed to find debug-variable for argument '" << argName
                  << "' in wrapper " << utils::getSrcNameOfFunction(irWrapper);
                throw utils::PallasException(s.str());
            }
            auto *wArg = nameToWArg.at(argName);
            // Build tuple of shape {ParentDIVar, WrapperDIVar}
            argMappingNodes.push_back(
                llvm::MDTuple::get(irContext, {pVar, wArg}));
        }
        return llvm::MDTuple::get(irContext, argMappingNodes);
    }

    llvm::MDNode *SpecEncoder::encodeGhostVarSet(
        llvm::ArrayRef<std::string> gVarNames,
        const llvm::Function &irWrapper,
        const MatchedFunctionContract *parentContract) const
    {
        if (gVarNames.empty())
            return llvm::MDTuple::get(irContext, {});

        if (parentContract == nullptr)
        {
            std::stringstream s;
            s << "Cannot determine ghost variables for function without contract ("
              << utils::getSrcNameOfFunction(*parentContract->function)
              << ")";
            throw utils::PallasException(s.str());
        }

        auto nameToVar = mapDIArgsToNames(irWrapper);

        llvm::SmallVector<llvm::Metadata *> nodes;
        for (auto gName : gVarNames)
        {

            // Get MD-Node that encodes the ghost variable
            auto *vDef = parentContract->getGhostArg(gName);
            if (vDef == nullptr)
            {
                std::stringstream s;
                s << "Failed to find the definition of ghost variable " << gName
                  << " in the contract of function "
                  << utils::getSrcNameOfFunction(*parentContract->function);
                throw utils::PallasException(s.str());
            }
            if (!ghostArgDefMap.contains(vDef))
            {
                std::stringstream s;
                s << "Failed to find the encoding of ghost variable " << gName
                  << " while building the argument-list of wrapper function "
                  << utils::getSrcNameOfFunction(irWrapper);
                throw utils::PallasException(s.str());
            }
            auto *vDefMD = ghostArgDefMap.at(vDef);

            // Get DIVariable that corresponds to the arg of the wrapper
            if (!nameToVar.contains(gName))
            {
                std::stringstream s;
                s << "Failed to find ghost variable " << gName
                  << " in the debug-info of wrapper-function "
                  << utils::getSrcNameOfFunction(irWrapper);
                throw utils::PallasException(s.str());
            }
            auto *wDIVar = nameToVar.at(gName);

            // Build tuple: {VAR_DEF, DIVar}
            auto *entry = llvm::MDTuple::get(irContext, {vDefMD, wDIVar});
            nodes.push_back(entry);
        }
        return llvm::MDTuple::get(irContext, nodes);
    }

    llvm::MDTuple *SpecEncoder::encodeSrcLoc(const SrcRange &srcLoc) const
    {
        llvm::DIBuilder builder(irModule);
        auto absPath = utils::makeAbsPath(srcLoc.getFile());
        llvm::StringRef checksumStr = srcLoc.getMD5();
        llvm::DIFile::ChecksumInfo checksum(
            llvm::DIFile::ChecksumKind::CSK_MD5, checksumStr);
        auto *file = builder.createFile(absPath, "", checksum);
        builder.finalize();
        auto *nTy = llvm::Type::getInt64Ty(irContext);

        llvm::SmallVector<llvm::Metadata *> mdElems = {
            llvm::MDString::get(irContext, PALLAS_SRC_LOCATION_MARKER),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(nTy, srcLoc.startLoc.line)),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(nTy, srcLoc.startLoc.column)),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(nTy, srcLoc.endLoc.line)),
            llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::get(nTy, srcLoc.endLoc.column)),
            file};

        return llvm::MDTuple::get(irContext, mdElems);
    }

    llvm::StringMap<llvm::DILocalVariable *>
    SpecEncoder::mapDIArgsToNames(const llvm::Function &f) const
    {
        llvm::StringMap<llvm::DILocalVariable *> namesToVars;
        for (auto it = llvm::inst_begin(f), end = llvm::inst_end(f);
             it != end; ++it)
        {
            if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*it))
            {
                auto *var = intr->getVariable();
                auto name = var->getName();
                if (var->getArg() != 0 && !namesToVars.contains(name))
                    namesToVars.insert({name, var});
            }
        }

        return namesToVars;
    }

    void SpecEncoder::throwNoContractForFunc(const llvm::Function &func)
    {
        std::stringstream s;
        s << "Failed to find contract for function"
          << utils::getSrcNameOfFunction(func);
        throw utils::PallasException(s.str());
    }
}