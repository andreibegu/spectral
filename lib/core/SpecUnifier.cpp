#include "SpecUnifier.h"
#include "IrSpecConstants.h"
#include "MetadataUtils.h"
#include "PallasException.h"

#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

namespace sourcespec2ir
{

    SpecUnifier::SpecUnifier(const bool printDebugInfo)
        : printDebugInfo(printDebugInfo)
    {
    }

    void SpecUnifier::unifySpecifications(llvm::Module *irModule,
                                          llvm::LLVMContext *irContext)
    {
        // TODO: Move the arguments to the constructor
        this->irModule = irModule;
        this->irContext = irContext;

        // Replace all allocations that use a type from the source spec-library
        replaceSpecTypes();

        // Collect all used specification-functions from the module
        llvm::SmallVector<llvm::Function *, 32> srcSpecFuns;
        for (auto &f : irModule->getFunctionList())
        {
            if (isSrcSpecificationFunction(f).has_value())
            {
                srcSpecFuns.push_back(&f);
            }
        }

        // Replace all specification functions:
        while (!srcSpecFuns.empty())
        {
            auto srcSpecFun = srcSpecFuns.pop_back_val();

            // Replace all calls to the specification.
            replaceCallsToSpecFunction(*srcSpecFun);

            if (!srcSpecFun->users().empty())
            {
                auto srcSpecFunName = srcSpecFun->getName().str();
                throw utils::PallasException(
                    "Failed to remove specification function (" +
                    srcSpecFunName +
                    "). Remaining users exist.");
            }

            // Remove the function from the module
            srcSpecFun->eraseFromParent();
        }
    }

    llvm::CallInst *SpecUnifier::transformImply(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecImply(), newArgs);
    }

    llvm::CallInst *SpecUnifier::transformScAnd(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecScAnd(), newArgs);
    }

    llvm::CallInst *SpecUnifier::transformScOr(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecScOr(), newArgs);
    }

    llvm::CallInst *SpecUnifier::transformSepConj(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecSepConj(), newArgs);
    }

    llvm::CallInst *SpecUnifier::transformOld(llvm::CallInst &oldCall)
    {
        auto numArgs = oldCall.arg_size();

        // Build representations for return-value and passed value
        auto retRepr = IRTypeRepresentation::getRetReprFromCall(oldCall);
        auto argRepr = retRepr->isReturnByParam()
                           ? IRTypeRepresentation::getArgReprFromCall(oldCall, 1, numArgs - 1)
                           : IRTypeRepresentation::getArgReprFromCall(oldCall, 0, numArgs);

        // Build new arg-list (1:1 copy)
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < numArgs; ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        // Build new call
        auto newCall = llvm::CallInst::Create(
            getUSpecOld(*retRepr, *argRepr), newArgs);

        // Add attributes to new call
        applyAttrToReturn(*retRepr, newCall);
        applyAttrToParamRepr(*argRepr, newCall, retRepr->isReturnByParam() ? 1 : 0);

        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformResult(llvm::CallInst &oldCall)
    {
        auto numArgs = oldCall.arg_size();
        // Build representation of the return-type
        auto retRepr = IRTypeRepresentation::getRetReprFromCall(oldCall);

        // Build new arg-list (1:1 copy)
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < numArgs; ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        auto newCall = llvm::CallInst::Create(
            getUSpecResult(*retRepr), newArgs);

        // Copy attributes to new call
        applyAttrToReturn(*retRepr, newCall);

        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformBoundVar(llvm::CallInst &oldCall)
    {
        auto retRepr = IRTypeRepresentation::getRetReprFromCall(oldCall);

        // Build new arg-list (1:1 copy)
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < oldCall.arg_size(); ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        auto newCall = llvm::CallInst::Create(getUSpecBoundVar(*retRepr), newArgs);
        applyAttrToReturn(*retRepr, newCall);
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformExists(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecExists(), newArgs);
    }

    llvm::CallInst *SpecUnifier::transformForall(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecForall(), newArgs);
    }

    llvm::CallInst *SpecUnifier::transformForallSep(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecForallSep(), newArgs);
    }

    llvm::CallInst *SpecUnifier::transformFracOf(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 4> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1),
            oldCall.getArgOperand(2)};
        auto newCall = llvm::CallInst::Create(getUSpecFracOf(), newArgs);
        addSret(newCall, 0, getUSpecFracType());
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        newCall->addParamAttr(2, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformPerm(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        auto newCall = llvm::CallInst::Create(getUSpecPerm(), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        addByVal(newCall, 1, getUSpecFracType());
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformPtrBlockLength(llvm::CallInst &oldCall)
    {
        auto newCall = llvm::CallInst::Create(
            getUSpecPtrBlockLength(),
            {oldCall.getArgOperand(0)});
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformPtrBlockOffset(llvm::CallInst &oldCall)
    {
        auto newCall = llvm::CallInst::Create(
            getUSpecPtrBlockOffset(),
            {oldCall.getArgOperand(0)});
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformPtrLength(llvm::CallInst &oldCall)
    {
        auto newCall = llvm::CallInst::Create(
            getUSpecPtrLength(),
            {oldCall.getArgOperand(0)});
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformUnfolding(llvm::CallInst &oldCall)
    {
        auto numArgs = oldCall.arg_size();

        auto retRepr = IRTypeRepresentation::getRetReprFromCall(oldCall);

        // Check that first arg is boolean for the pred-expr:
        // TODO: Implement

        auto exprRepr = retRepr->isReturnByParam()
                            ? IRTypeRepresentation::getArgReprFromCall(oldCall, 2, numArgs - 2)
                            : IRTypeRepresentation::getArgReprFromCall(oldCall, 1, numArgs - 1);

        // Build new arg-list (1:1 copy)
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < numArgs; ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        // Build new call
        auto *newFunc = getUSpecUnfolding(*retRepr, *exprRepr);
        auto newCall = llvm::CallInst::Create(newFunc, newArgs);

        // Add attributes to new call
        applyAttrToReturn(*retRepr, newCall);
        applyAttrToParamRepr(*exprRepr, newCall, retRepr->isReturnByParam() ? 2 : 1);

        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformSeqNew(llvm::CallInst &oldCall)
    {
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
        llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
        auto newCall = llvm::CallInst::Create(getUSpecSeqNew(cType), newArgs);
        addSret(newCall, 0, getUSpecSeqType(cType));
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformSeqSize(llvm::CallInst &oldCall)
    {
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
        llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
        auto newCall = llvm::CallInst::Create(getUSpecSeqSize(cType), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        addByVal(newCall, 0, getUSpecSeqType(cType));
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformSeqGet(llvm::CallInst &oldCall)
    {
        auto numArgs = oldCall.getNumOperands() - 1;

        // Build type-representation of the return-type
        auto retRepr = IRTypeRepresentation::getRetReprFromCall(oldCall);

        // Check if the result is returned as an sret-parameter
        auto isParamReturn = retRepr->isReturnByParam();

        // Get the content-type of the sequence from the argument
        auto cType = getContentTypeOfADT(
            *oldCall.getArgOperand(isParamReturn ? 1 : 0));

        // Build new arg-list by copying arguments 1:1
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < numArgs; ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        // Build the new call
        auto newCall = llvm::CallInst::Create(
            getUSpecSeqGet(cType, *retRepr), newArgs);

        // Add attributes:
        applyAttrToReturn(*retRepr, newCall);
        newCall->addParamAttr(isParamReturn ? 1 : 0, llvm::Attribute::NoUndef);
        addByVal(newCall, isParamReturn ? 1 : 0, getUSpecSeqType(cType));
        newCall->addParamAttr(isParamReturn ? 2 : 1, llvm::Attribute::NoUndef);
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformSeqPrepend(llvm::CallInst &oldCall)
    {
        // We assume, that the first argument is the return-value with an
        // sret-attribute, the last argument is the pointer to the passed-in
        // sequence, and the arguments in between represent the element that is
        // prepended.
        // In most cases, this will only be one argument, but e.g. Swift and
        // Rust pass small structs as their individual elements.
        auto numArgs = oldCall.arg_size();

        // Obtain the content-type from the sequence that is passed as the first arg.
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(numArgs - 1));
        auto seqType = getUSpecSeqType(cType);

        // Copy types and arguments of the passed element's representation.
        auto elemTRepr = IRTypeRepresentation::getArgReprFromCall(oldCall, 1, numArgs - 2);

        // Copy arguments 1:1 to the new call
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < numArgs; ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        auto newCall = llvm::CallInst::Create(
            getUSpecSeqPrepend(cType, *elemTRepr), newArgs);

        // Add attributes to return-type
        addSret(newCall, 0, seqType);
        // Add Attributes to representation of the prepended element
        applyAttrToParamRepr(*elemTRepr, newCall, 1);
        // Add attributes for the passed-in sequence
        newCall->addParamAttr(numArgs - 1, llvm::Attribute::NoUndef);
        addByVal(newCall, numArgs - 1, seqType);

        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformSeqEquals(llvm::CallInst &oldCall)
    {
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
        auto seqType = getUSpecSeqType(cType);
        llvm::SmallVector<llvm::Value *> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1),
        };
        auto newCall = llvm::CallInst::Create(getUSpecSeqEquals(cType), newArgs);
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        addByVal(newCall, 0, seqType);
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        addByVal(newCall, 1, seqType);
        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformSeqSlice(llvm::CallInst &oldCall)
    {
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(1));
        auto seqType = getUSpecSeqType(cType);
        llvm::SmallVector<llvm::Value *> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1),
            oldCall.getArgOperand(2),
            oldCall.getArgOperand(3),
        };
        auto newCall = llvm::CallInst::Create(getUSpecSeqSlice(cType), newArgs);

        // Add attributes
        newCall->addParamAttr(0, llvm::Attribute::NoUndef);
        addSret(newCall, 0, seqType);
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        addByVal(newCall, 1, seqType);

        return newCall;
    }

    llvm::CallInst *SpecUnifier::transformSeqSet(llvm::CallInst &oldCall)
    {
        auto numArgs = oldCall.getNumOperands() - 1;
        auto argRepr = IRTypeRepresentation::getArgReprFromCall(oldCall, 3, numArgs - 3);

        // Get the content-type of the sequence from the argument
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(1));
        auto seqType = getUSpecSeqType(cType);

        // Build new arg-list by copying arguments 1:1
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < numArgs; ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        // Build the new call
        auto newCall = llvm::CallInst::Create(
            getUSpecSeqSet(cType, *argRepr), newArgs);

        // Add attributes:
        addSret(newCall, 0, seqType);
        newCall->addParamAttr(1, llvm::Attribute::NoUndef);
        addByVal(newCall, 1, seqType);
        return newCall;
    }

    llvm::Type *SpecUnifier::getContentTypeOfADT(const llvm::Value &v)
    {
        // Get the content-type of the ADT from the first element of the struct
        // Currently, only types allocated via alloca are supported.
        // TODO: Expand this (arguments, ...)
        if (auto *allocInstr = llvm::dyn_cast<llvm::AllocaInst>(&v))
        {
            return getContentTypeOfADTTy(*allocInstr->getAllocatedType());
        }
        else if (auto *arg = llvm::dyn_cast<llvm::Argument>(&v))
        {
            if (arg->hasByValAttr())
                return getContentTypeOfADTTy(*arg->getParamByValType());
            if (arg->hasByRefAttr())
                return getContentTypeOfADTTy(*arg->getParamByRefType());
            if (arg->hasStructRetAttr())
                return getContentTypeOfADTTy(*arg->getParamStructRetType());
        }
        throw utils::PallasException("Failed to extract the content-type of the the ADT.");
    }

    void SpecUnifier::replaceCallsToSpecFunction(llvm::Function &srcSpecFunction)
    {
        while (!srcSpecFunction.user_empty())
        {
            llvm::User *user = *srcSpecFunction.user_begin();
            if (printDebugInfo)
            {
                llvm::outs() << "===============================================\n";
                llvm::outs() << "Detected user of "
                             << srcSpecFunction.getName().str() << ":\n";
                user->print(llvm::outs());
                llvm::outs() << "\n";
            }

            auto oldCall = llvm::dyn_cast<llvm::CallInst>(user);
            if (!oldCall)
            {
                // Not a function call
                throw utils::PallasException(
                    "Encountered a use of a specification-function that is not a call.");
            }
            auto parentF = oldCall->getParent()->getParent();
            // Calls to spec-functions are only allowed from within wrapper-functions
            if (!utils::isGeneratedSpecFunction(*parentF) &&
                !utils::isGhostWrapper(*parentF) &&
                !utils::isPredicateDef(*parentF))
            {
                throw utils::PallasException(
                    "Encountered a call to a specification-function outside of a specification.");
            }

            auto newCall = transformSpecCall(*oldCall);
            if (!newCall)
            {
                auto srcSpecName = oldCall->getCalledFunction()->getName().str();
                throw utils::PallasException("Failed to generate a new call to replace use of " + srcSpecName);
            }

            // Replace old call
            if (!oldCall->use_empty())
            {
                oldCall->replaceAllUsesWith(newCall);
            }
            llvm::ReplaceInstWithInst(oldCall, newCall);
        }
    }

    llvm::CallInst *SpecUnifier::transformSpecCall(llvm::CallInst &oldCall)
    {
        const auto specCallType = isSrcSpecificationFunction(*oldCall.getCalledFunction());
        if (!specCallType.has_value())
        {
            auto fName = oldCall.getCalledFunction()->getName().str();
            throw utils::PallasException(
                "Failed to identify call to specification library. Called function is " + fName);
        }

        switch (specCallType.value())
        {
        case Imply:
            return transformImply(oldCall);
        case ScAnd:
            return transformScAnd(oldCall);
        case ScOr:
            return transformScOr(oldCall);
        case SepConj:
            return transformSepConj(oldCall);
        case Old:
            return transformOld(oldCall);
        case Result:
            return transformResult(oldCall);
        case BoundVar:
            return transformBoundVar(oldCall);
        case Exists:
            return transformExists(oldCall);
        case Forall:
            return transformForall(oldCall);
        case ForallSep:
            return transformForallSep(oldCall);
        case FracOf:
        {
            auto newCall = transformFracOf(oldCall);
            // Check that the type for fractions has been replaced.
            if (auto *allocInstr = llvm::dyn_cast<llvm::AllocaInst>(oldCall.getArgOperand(0)))
            {
                if (allocInstr->getAllocatedType() != getUSpecFracType())
                {
                    throw utils::PallasException("The spec-lib type for fractions has not been replaced properly.");
                }
            }
            else
            {
                throw utils::PallasException("Allocation for fractions is only supported using alloca.");
            }
            return newCall;
        }
        case Perm:
            return transformPerm(oldCall);
        case PtrBlockLength:
            return transformPtrBlockLength(oldCall);
        case PtrBlockOffset:
            return transformPtrBlockOffset(oldCall);
        case PtrLength:
            return transformPtrLength(oldCall);
        case Unfolding:
            return transformUnfolding(oldCall);
        case SeqNew:
            return transformSeqNew(oldCall);
        case SeqSize:
            return transformSeqSize(oldCall);
        case SeqGet:
            return transformSeqGet(oldCall);
        case SeqPrepend:
            return transformSeqPrepend(oldCall);
        case SeqEquals:
            return transformSeqEquals(oldCall);
        case SeqSlice:
            return transformSeqSlice(oldCall);
        case SeqSet:
            return transformSeqSet(oldCall);
        default:
            throw utils::PallasException(
                "Encountered an unsupported type of specification-call.");
        }
    }

    void SpecUnifier::replaceSpecTypes()
    {
        auto *srcFracType = getSrcLibFracType();

        // Collect all of the sequence-types that are used.
        llvm::SmallPtrSet<llvm::StructType *, 8> seqTypes;
        for (auto st : irModule->getIdentifiedStructTypes())
        {
            if (isSrcSeqType(*st))
            {
                seqTypes.insert(st);
            }
        }

        // If no types from the source-speclib are used, skip
        if (srcFracType == nullptr && seqTypes.empty() && printDebugInfo)
        {
            llvm::outs() << "Skipping replacement of spec-types. \n";
            return;
        }

        for (auto &f : irModule->getFunctionList())
        {
            // Skip all functions that do not contain generated specifications.
            if (!utils::isGeneratedSpecFunction(f) &&
                !utils::isGhostWrapper(f) &&
                !utils::isPredicateDef(f))
                continue;

            // Update attributes of function-arguments
            for (auto &arg : f.args())
            {
                // ByVal
                if (auto *sTy = llvm::dyn_cast_if_present<llvm::StructType>(arg.getParamByValType()))
                {
                    if (seqTypes.contains(sTy))
                    {
                        auto *newTy = getUSpecSeqType(getContentTypeOfADTTy(*sTy));
                        arg.removeAttr(llvm::Attribute::ByVal);
                        arg.addAttr(llvm::Attribute::getWithByValType(*irContext, newTy));
                    }
                }
                // ByRef
                if (auto *sTy = llvm::dyn_cast_if_present<llvm::StructType>(arg.getParamByRefType()))
                {
                    if (seqTypes.contains(sTy))
                    {
                        auto *newTy = getUSpecSeqType(getContentTypeOfADTTy(*sTy));
                        arg.removeAttr(llvm::Attribute::ByRef);
                        arg.addAttr(llvm::Attribute::getWithByRefType(*irContext, newTy));
                    }
                }
                // sret
                if (auto *sTy = llvm::dyn_cast_if_present<llvm::StructType>(arg.getParamStructRetType()))
                {
                    if (seqTypes.contains(sTy))
                    {
                        auto *newTy = getUSpecSeqType(getContentTypeOfADTTy(*sTy));
                        arg.removeAttr(llvm::Attribute::StructRet);
                        arg.addAttr(llvm::Attribute::getWithStructRetType(*irContext, newTy));
                    }
                }
            }

            // Check all instructions in the function
            for (llvm::inst_iterator it = llvm::inst_begin(f), end = llvm::inst_end(f); it != end; ++it)
            {
                // Filter for alloca-instructions
                if (auto *allocInstr = llvm::dyn_cast<llvm::AllocaInst>(&*it))
                {
                    llvm::Type *newType = nullptr;
                    auto oldType = allocInstr->getAllocatedType();

                    // Handle fraction-type
                    if (oldType == srcFracType)
                    {
                        newType = getUSpecFracType();
                    }
                    else if (auto *sType = llvm::dyn_cast<llvm::StructType>(oldType))
                    {
                        // Handle sequence-types
                        if (seqTypes.contains(sType))
                        {
                            auto contentType = getContentTypeOfADTTy(*sType);
                            newType = getUSpecSeqType(contentType);
                        }
                    }

                    if (newType != nullptr)
                    {
                        // TODO: Repace call to setAllocatedType with something better (see doxygen)
                        allocInstr->setAllocatedType(newType);
                    }
                }
                else if (auto *call = llvm::dyn_cast<llvm::CallBase>(&*it))
                {
                    // TODO: Also handle types in byref / sret / byval attributes of calls!
                    for (unsigned int argIdx = 0; argIdx < call->arg_size(); ++argIdx)
                    {
                        // ByVal
                        if (call->paramHasAttr(argIdx, llvm::Attribute::ByVal))
                        {
                            auto *attrType = llvm::dyn_cast_if_present<llvm::StructType>(
                                call->getAttributes().getParamByValType(argIdx));
                            if (attrType == nullptr)
                                continue;
                            if (seqTypes.contains(attrType))
                            {
                                auto *newTy = getUSpecSeqType(getContentTypeOfADTTy(*attrType));
                                call->removeParamAttr(argIdx, llvm::Attribute::ByVal);
                                call->addParamAttr(argIdx,
                                                   llvm::Attribute::getWithByValType(*irContext, newTy));
                            }
                        }
                        // ByRef
                        else if (call->paramHasAttr(argIdx, llvm::Attribute::ByRef))
                        {
                            auto *attrType = llvm::dyn_cast_if_present<llvm::StructType>(
                                call->getAttributes().getParamByRefType(argIdx));
                            if (attrType == nullptr)
                                continue;
                            if (seqTypes.contains(attrType))
                            {
                                auto *newTy = getUSpecSeqType(getContentTypeOfADTTy(*attrType));
                                call->removeParamAttr(argIdx, llvm::Attribute::ByRef);
                                call->addParamAttr(argIdx,
                                                   llvm::Attribute::getWithByRefType(*irContext, newTy));
                            }
                            // Sret
                        }
                        else if (call->paramHasAttr(argIdx, llvm::Attribute::StructRet))
                        {
                            auto *attrType = llvm::dyn_cast_if_present<llvm::StructType>(
                                call->getAttributes().getParamStructRetType(argIdx));
                            if (attrType == nullptr)
                                continue;
                            if (seqTypes.contains(attrType))
                            {
                                auto *newTy = getUSpecSeqType(getContentTypeOfADTTy(*attrType));
                                call->removeParamAttr(argIdx, llvm::Attribute::StructRet);
                                call->addParamAttr(argIdx,
                                                   llvm::Attribute::getWithStructRetType(*irContext, newTy));
                            }
                        }
                    }
                }
            }
        }
    }

    llvm::StructType *SpecUnifier::getUSpecFracType()
    {
        auto fracType = llvm::StructType::getTypeByName(*irContext, PALLAS_SPEC_TYPE_FRAC);
        if (fracType == nullptr)
        {
            // Implement as identified struct-type ( could be made potentially opaque?!)
            // The concrete contents of the type do not matter, only used as a 'placeholder'.
            llvm::SmallVector<llvm::Type *, 4> elems = {
                llvm::Type::getInt64Ty(*irContext), llvm::Type::getInt64Ty(*irContext),
                llvm::Type::getInt64Ty(*irContext), llvm::Type::getInt64Ty(*irContext)};
            fracType = llvm::StructType::create(elems, PALLAS_SPEC_TYPE_FRAC);
        }
        return fracType;
    }

    namespace
    {
        std::string getTypeAsStr(const llvm::Type *type)
        {
            std::string tName;
            llvm::raw_string_ostream nameStr(tName);
            type->print(nameStr, false, true);
            return tName;
        }
    }

    llvm::StructType *SpecUnifier::getUSpecSeqType(llvm::Type *type)
    {
        if (specSeqTypes.find(type) == specSeqTypes.end())
        {
            // Generate declaration for new sequence-type
            std::string tName = PALLAS_SPEC_TYPE_SEQ_BASE + "." +
                                getTypeAsStr(type);

            // The first value in the sequence type determines the content-type of the sequence
            // The remaining values are dummy-values to increase the size which prevents
            // ABI optimizations when the struct is passed to a function.
            llvm::SmallVector<llvm::Type *, 4> elems = {
                type,
                llvm::Type::getInt64Ty(*irContext),
                llvm::Type::getInt64Ty(*irContext),
                llvm::Type::getInt64Ty(*irContext)};
            auto seqT = llvm::StructType::create(elems, tName);
            specSeqTypes[type] = seqT;
        }

        return specSeqTypes[type];
    }

    llvm::Function *SpecUnifier::getUSpecImply()
    {
        if (!specImply)
        {
            // Generate and add declaration of imply-function
            // TODO: Check if zeroext, noundef attributes should be added
            llvm::SmallVector<llvm::Type *> implySpecArgs(
                {llvm::Type::getInt1Ty(*irContext), llvm::Type::getInt1Ty(*irContext)});
            auto implySpecFT = llvm::FunctionType::get(
                llvm::Type::getInt1Ty(*irContext), implySpecArgs, false);
            specImply = llvm::Function::Create(
                implySpecFT, llvm::Function::ExternalLinkage, PALLAS_SPEC_IMPLY, irModule);

            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specImply, PALLAS_SPEC_IMPLY);
        }
        return specImply;
    }

    llvm::Function *SpecUnifier::getUSpecScAnd()
    {
        if (!specScAnd)
        {
            // TODO: Check if zeroext, noundef attributes should be added
            llvm::SmallVector<llvm::Type *> specArgs(
                {llvm::Type::getInt1Ty(*irContext), llvm::Type::getInt1Ty(*irContext)});
            auto specFT = llvm::FunctionType::get(
                llvm::Type::getInt1Ty(*irContext), specArgs, false);
            specScAnd = llvm::Function::Create(
                specFT, llvm::Function::ExternalLinkage, PALLAS_SPEC_SCAND, irModule);

            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specScAnd, PALLAS_SPEC_SCAND);
        }
        return specScAnd;
    }

    llvm::Function *SpecUnifier::getUSpecScOr()
    {
        if (!specScOr)
        {
            // TODO: Check if zeroext, noundef attributes should be added
            llvm::SmallVector<llvm::Type *> specArgs(
                {llvm::Type::getInt1Ty(*irContext), llvm::Type::getInt1Ty(*irContext)});
            auto specFT = llvm::FunctionType::get(
                llvm::Type::getInt1Ty(*irContext), specArgs, false);
            specScOr = llvm::Function::Create(
                specFT, llvm::Function::ExternalLinkage, PALLAS_SPEC_SCOR, irModule);

            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specScOr, PALLAS_SPEC_SCOR);
        }
        return specScOr;
    }

    llvm::Function *SpecUnifier::getUSpecSepConj()
    {
        if (!specSepConj)
        {
            specSepConj = createSpecLibFunction(
                PALLAS_SPEC_SEPCONJ, getBoolTy(), {getBoolTy(), getBoolTy()});
        }
        return specSepConj;
    }

    llvm::Function *SpecUnifier::getUSpecOld(const IRTypeRepresentation &retRepr,
                                             const IRTypeRepresentation &argRepr)
    {
        // Check if an appropriate function has already been generated
        for (auto t : specOld)
        {
            if (std::get<0>(t) == retRepr && std::get<1>(t) == argRepr)
                return std::get<2>(t);
        }

        // Generate declaration for new old-function with the given type
        auto specName = PALLAS_SPEC_OLD_BASE + " " +
                        retRepr.asString() + "_" +
                        argRepr.asString();

        auto isParamRet = retRepr.isReturnByParam();
        llvm::SmallVector<llvm::Type *> argList;
        // Ptr for return value
        if (isParamRet)
            argList.push_back(llvm::PointerType::get(*irContext, 0));
        // Representation of passed element
        for (auto t : argRepr.getTypes())
            argList.push_back(t);

        auto specFun = createSpecLibFunction(
            specName, retRepr.getTypes().front(), argList, PALLAS_SPEC_OLD_BASE);

        // Add attributes:
        applyAttrToReturn(retRepr, specFun);
        applyAttrToParamRepr(argRepr, specFun, isParamRet ? 1 : 0);

        // Register the new spec-function
        // TODO: Get rid of duplicate copies
        specOld.push_back(std::make_tuple(IRTypeRepresentation(retRepr),
                                          IRTypeRepresentation(argRepr),
                                          specFun));

        return specFun;
    }

    llvm::Function *SpecUnifier::getUSpecResult(
        const IRTypeRepresentation &retRepr)
    {
        // Check if an appropriate function has already been generated
        for (auto p : specResult)
            if (p.first == retRepr)
                return p.second;

        // Generate declaration for new result-function with the given type
        auto specName = PALLAS_SPEC_RESULT_BASE + " " + retRepr.asString();

        llvm::SmallVector<llvm::Type *> argList;
        // Ptr for return value
        if (retRepr.isReturnByParam())
            argList.push_back(llvm::PointerType::get(*irContext, 0));

        auto specFun = createSpecLibFunction(
            specName, retRepr.getTypes().front(),
            argList, PALLAS_SPEC_RESULT_BASE);

        // Add attributes:
        applyAttrToReturn(retRepr, specFun);

        // Register the new spec-function:
        // Todo: Get rid of duplicate copies, and change to use a map instead of
        // a vector of pairs.
        specResult.push_back(
            std::make_pair(IRTypeRepresentation(retRepr), specFun));

        return specFun;
    }

    llvm::Function *SpecUnifier::getUSpecBoundVar(
        const IRTypeRepresentation &retRepr)
    {
        // Check if an appropriate function has already been generated
        for (auto p : specBoundVar)
            if (p.first == retRepr)
                return p.second;

        // Generate declaration for the given type
        auto specName = PALLAS_SPEC_BOUND_VAR + " " + retRepr.asString();

        llvm::SmallVector<llvm::Type *> argList;
        // Ptr for the return-value
        if (retRepr.isReturnByParam())
            argList.push_back(llvm::PointerType::get(*irContext, 0));
        // Ptr for the string-constant
        argList.push_back(llvm::PointerType::get(*irContext, 0));

        auto specFun = createSpecLibFunction(
            specName, retRepr.getTypes().front(), argList, PALLAS_SPEC_BOUND_VAR);

        // Apply attributes
        applyAttrToReturn(retRepr, specFun);

        specBoundVar.push_back(
            std::make_pair(IRTypeRepresentation(retRepr), specFun));
        return specFun;
    }

    llvm::Function *SpecUnifier::getUSpecExists()
    {
        if (!specExists)
        {
            specExists = createSpecLibFunction(PALLAS_SPEC_EXISTS,
                                               getBoolTy(), {getBoolTy(), getBoolTy()});
        }
        return specExists;
    }

    llvm::Function *SpecUnifier::getUSpecForall()
    {
        if (!specForall)
        {
            specForall = createSpecLibFunction(PALLAS_SPEC_FORALL,
                                               getBoolTy(), {getBoolTy(), getBoolTy()});
        }
        return specForall;
    }

    llvm::Function *SpecUnifier::getUSpecForallSep()
    {
        if (!specForallSep)
        {
            specForallSep = createSpecLibFunction(PALLAS_SPEC_FORALL_SEP,
                                                  getBoolTy(), {getBoolTy(), getBoolTy()});
        }
        return specForallSep;
    }

    llvm::Function *SpecUnifier::getUSpecFracOf()
    {
        if (!specFracOf)
        {

            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(*irContext, 0),
                llvm::Type::getInt32Ty(*irContext),
                llvm::Type::getInt32Ty(*irContext)};
            auto specFun = createSpecLibFunction(PALLAS_SPEC_FRAC_OF,
                                                 llvm::Type::getVoidTy(*irContext), argList);
            specFun->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecFracType()));
            specFun->addParamAttr(1, llvm::Attribute::NoUndef);
            specFun->addParamAttr(2, llvm::Attribute::NoUndef);

            specFracOf = specFun;
        }
        return specFracOf;
    }

    llvm::Function *SpecUnifier::getUSpecPerm()
    {
        if (!specPerm)
        {
            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(*irContext, 0),
                llvm::PointerType::get(getUSpecFracType(), 0)};
            auto specFun = createSpecLibFunction(PALLAS_SPEC_PERM, getBoolTy(), argList);
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specFun->addParamAttr(1, llvm::Attribute::NoUndef);
            specFun->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, getUSpecFracType()));
            specPerm = specFun;
        }
        return specPerm;
    }

    llvm::Function *SpecUnifier::getUSpecPtrBlockLength()
    {
        if (!specPtrBlockLength)
        {
            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(*irContext, 0)};
            auto specFun = createSpecLibFunction(
                PALLAS_SPEC_PTR_BLOCK_LENGTH,
                llvm::IntegerType::getInt64Ty(*irContext), argList);
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specPtrBlockLength = specFun;
        }
        return specPtrBlockLength;
    }

    llvm::Function *SpecUnifier::getUSpecPtrBlockOffset()
    {
        if (!specPtrBlockOffset)
        {
            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(*irContext, 0)};
            auto specFun = createSpecLibFunction(
                PALLAS_SPEC_PTR_BLOCK_OFFSET,
                llvm::IntegerType::getInt64Ty(*irContext), argList);
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specPtrBlockOffset = specFun;
        }
        return specPtrBlockOffset;
    }

    llvm::Function *SpecUnifier::getUSpecPtrLength()
    {
        if (!specPtrLength)
        {
            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(*irContext, 0)};
            auto specFun = createSpecLibFunction(
                PALLAS_SPEC_PTR_LENGTH,
                llvm::IntegerType::getInt64Ty(*irContext), argList);
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specPtrLength = specFun;
        }
        return specPtrLength;
    }

    llvm::Function *SpecUnifier::getUSpecUnfolding(
        const IRTypeRepresentation &retRepr,
        const IRTypeRepresentation &argRepr)
    {
        // Check if an appropriate function has already been generated
        for (auto t : specUnfolding)
        {
            if (std::get<0>(t) == retRepr && std::get<1>(t) == argRepr)
                return std::get<2>(t);
        }

        // Generate declaration for new function
        auto specName = PALLAS_SPEC_UNFOLDING_BASE + " " +
                        retRepr.asString() + "_" +
                        argRepr.asString();

        llvm::SmallVector<llvm::Type *> argList;
        // Ptr for return value
        if (retRepr.isReturnByParam())
            argList.push_back(llvm::PointerType::get(*irContext, 0));

        // Boolean for the predicate
        argList.push_back(llvm::Type::getInt1Ty(*irContext));

        // Representation of expression type
        for (auto t : argRepr.getTypes())
            argList.push_back(t);

        auto specFun = createSpecLibFunction(
            specName, retRepr.getTypes().front(), argList,
            PALLAS_SPEC_UNFOLDING_BASE);

        // Add attributes:
        applyAttrToReturn(retRepr, specFun);

        auto predIdx = retRepr.isReturnByParam() ? 1 : 0;
        specFun->addParamAttr(predIdx, llvm::Attribute::NoUndef);
        specFun->addParamAttr(predIdx, llvm::Attribute::ZExt);

        applyAttrToParamRepr(argRepr, specFun,
                             retRepr.isReturnByParam() ? 2 : 1);

        // Register the new spec-function
        specUnfolding.push_back(std::make_tuple(IRTypeRepresentation(retRepr),
                                                IRTypeRepresentation(argRepr),
                                                specFun));
        return specFun;
    }

    llvm::Function *SpecUnifier::getUSpecSeqNew(llvm::Type *type)
    {
        if (specSeqNew.find(type) == specSeqNew.end())
        {
            // Generate function declaration
            auto specName = PALLAS_SPEC_SEQ_NEW_BASE + " " + getTypeAsStr(type);
            auto retType = llvm::PointerType::get(getUSpecSeqType(type), 0);
            auto specFun = createSpecLibFunction(specName, getVoidTy(), {retType}, PALLAS_SPEC_SEQ_NEW_BASE);
            specFun->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(type)));
            specSeqNew[type] = specFun;
        }

        return specSeqNew[type];
    }

    llvm::Function *SpecUnifier::getUSpecSeqSize(llvm::Type *type)
    {
        if (specSeqSize.find(type) == specSeqSize.end())
        {
            // Generate function declaration
            auto name = PALLAS_SPEC_SEQ_SIZE_BASE + " " + getTypeAsStr(type);
            auto argType = llvm::PointerType::get(getUSpecSeqType(type), 0);
            auto specFun = createSpecLibFunction(name, llvm::Type::getInt64Ty(*irContext), {argType}, PALLAS_SPEC_SEQ_SIZE_BASE);
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specFun->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specSeqSize[type] = specFun;
        }

        return specSeqSize[type];
    }

    llvm::Function *SpecUnifier::getUSpecSeqGet(
        llvm::Type *type, const IRTypeRepresentation &retRepr)
    {
        // Check if an appropriate function has already been generated
        for (auto t : specSeqGet)
            if (std::get<0>(t) == type && std::get<1>(t) == retRepr)
                return std::get<2>(t);

        // TODO: Change from here!
        if (retRepr.getTypes().size() != 1)
            throw utils::PallasException("Received more than one return-type");

        auto name = PALLAS_SPEC_SEQ_GET_BASE + " " +
                    getTypeAsStr(type) + "_" +
                    retRepr.asString();

        // Check if the result is returned directly, or as a sret-parameter.
        auto isParamReturn = retRepr.isReturnByParam();

        // Build argument-list
        llvm::SmallVector<llvm::Type *> argList;
        if (isParamReturn)
            argList.push_back(llvm::PointerType::get(*irContext, 0));
        argList.push_back(llvm::PointerType::get(getUSpecSeqType(type), 0));
        argList.push_back(llvm::Type::getInt64Ty(*irContext));
        // Build new function
        auto specFun = createSpecLibFunction(
            name, retRepr.getTypes().front(), argList, PALLAS_SPEC_SEQ_GET_BASE);
        // Add attributes
        applyAttrToReturn(retRepr, specFun);
        specFun->addParamAttr(isParamReturn ? 1 : 0, llvm::Attribute::NoUndef);
        specFun->addParamAttr(isParamReturn ? 1 : 0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
        specFun->addParamAttr(isParamReturn ? 2 : 1, llvm::Attribute::NoUndef);

        specSeqGet.push_back(std::make_tuple(
            type, IRTypeRepresentation(retRepr), specFun));
        return specFun;
    }

    llvm::Function *SpecUnifier::getUSpecSeqPrepend(
        llvm::Type *type, const IRTypeRepresentation &argRepr)
    {
        // Check if an appropriate function has already been generated
        for (auto t : specSeqPrepend)
            if (std::get<0>(t) == type && std::get<1>(t) == argRepr)
                return std::get<2>(t);

        auto name = PALLAS_SPEC_SEQ_PREPEND_BASE + " " +
                    getTypeAsStr(type) + "_" +
                    argRepr.asString();

        llvm::SmallVector<llvm::Type *> argList;
        // Pointer for the return-value
        argList.push_back(llvm::PointerType::get(*irContext, 0));
        // Representation of the element that is prepended
        for (auto t : argRepr.getTypes())
            argList.push_back(t);
        // Pointer for the passed-in sequence
        argList.push_back(llvm::PointerType::get(*irContext, 0));

        auto specFun = createSpecLibFunction(
            name, getVoidTy(), argList, PALLAS_SPEC_SEQ_PREPEND_BASE);

        // Add attributes:
        // Return-type
        specFun->addParamAttr(
            0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(type)));
        // Attributes of type-representation
        applyAttrToParamRepr(argRepr, specFun, 1);
        // Passed-in sequence
        specFun->addParamAttr(argList.size() - 1, llvm::Attribute::NoUndef);
        specFun->addParamAttr(
            argList.size() - 1,
            llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));

        specSeqPrepend.push_back(std::make_tuple(
            type, IRTypeRepresentation(argRepr), specFun));
        return specFun;
    }

    llvm::Function *SpecUnifier::getUSpecSeqEquals(llvm::Type *type)
    {
        if (specSeqEquals.find(type) == specSeqEquals.end())
        {
            auto name = PALLAS_SPEC_SEQ_EQUALS_BASE + " " + getTypeAsStr(type);

            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(*irContext, 0),
                llvm::PointerType::get(*irContext, 0)};

            auto specFun = createSpecLibFunction(
                name, getBoolTy(), argList, PALLAS_SPEC_SEQ_EQUALS_BASE);
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specFun->addParamAttr(0, llvm::Attribute::getWithByValType(
                                         *irContext, getUSpecSeqType(type)));
            specFun->addParamAttr(1, llvm::Attribute::NoUndef);
            specFun->addParamAttr(1, llvm::Attribute::getWithByValType(
                                         *irContext, getUSpecSeqType(type)));

            specSeqEquals[type] = specFun;
        }
        return specSeqEquals[type];
    }

    llvm::Function *SpecUnifier::getUSpecSeqSlice(llvm::Type *type)
    {
        if (specSeqSlice.find(type) == specSeqSlice.end())
        {
            auto name = PALLAS_SPEC_SEQ_SLICE_BASE + " " + getTypeAsStr(type);

            // Build arg-list
            llvm::SmallVector<llvm::Type *> argList;
            // Pointer for the return-value
            argList.push_back(llvm::PointerType::get(*irContext, 0));
            // Sequence
            argList.push_back(llvm::PointerType::get(*irContext, 0));
            // Start-index
            argList.push_back(llvm::Type::getInt64Ty(*irContext));
            // End-index
            argList.push_back(llvm::Type::getInt64Ty(*irContext));

            auto specFun = createSpecLibFunction(
                name, getVoidTy(), argList, PALLAS_SPEC_SEQ_SLICE_BASE);

            // Add attributes
            // Return-type
            specFun->addParamAttr(
                0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(type)));
            // Passed-in sequence
            specFun->addParamAttr(1, llvm::Attribute::NoUndef);
            specFun->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specSeqSlice[type] = specFun;
        }
        return specSeqSlice[type];
    }

    llvm::Function *SpecUnifier::getUSpecSeqSet(llvm::Type *type,
                                                const IRTypeRepresentation &argRepr)
    {
        // Check if an appropriate function has already been generated
        for (auto t : specSeqSet)
            if (std::get<0>(t) == type && std::get<1>(t) == argRepr)
                return std::get<2>(t);

        auto name = PALLAS_SPEC_SEQ_SET_BASE + " " +
                    getTypeAsStr(type) + "_" +
                    argRepr.asString();

        // Build arg-list
        llvm::SmallVector<llvm::Type *> argList;
        // Pointer for the return-value
        argList.push_back(llvm::PointerType::get(*irContext, 0));
        // Pointer to passed-in sequence
        argList.push_back(llvm::PointerType::get(*irContext, 0));
        // Insert-index
        argList.push_back(llvm::Type::getInt64Ty(*irContext));
        // Type-representation of value
        for (auto t : argRepr.getTypes())
            argList.push_back(t);

        auto specFun = createSpecLibFunction(
            name, getVoidTy(), argList, PALLAS_SPEC_SEQ_SET_BASE);

        // Add attributes:
        // sret-pointer
        specFun->addParamAttr(
            0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(type)));
        // Passed-in sequence
        specFun->addParamAttr(1, llvm::Attribute::NoUndef);
        specFun->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
        // Attributes of type-representation
        applyAttrToParamRepr(argRepr, specFun, 3);

        specSeqSet.push_back(std::make_tuple(
            type, IRTypeRepresentation(argRepr), specFun));
        return specFun;
    }

    llvm::Function *SpecUnifier::createSpecLibFunction(
        const std::string &name, llvm::Type *returnType,
        llvm::SmallVector<llvm::Type *> argTypes)
    {
        return createSpecLibFunction(name, returnType, argTypes, name);
    }

    llvm::Function *SpecUnifier::createSpecLibFunction(
        const std::string &name, llvm::Type *returnType,
        llvm::SmallVector<llvm::Type *> argTypes, const std::string &labelName)
    {
        auto specFunType = llvm::FunctionType::get(returnType, argTypes, false);
        auto specFunction = llvm::Function::Create(
            specFunType, llvm::Function::ExternalLinkage, name, irModule);
        // Add metadata-marker to mark as specification-library-function
        markAsSpecLibFunction(specFunction, labelName);
        return specFunction;
    }

    void SpecUnifier::markAsSpecLibFunction(llvm::Function *specLib,
                                            const std::string &identifier)
    {
        auto idStr = llvm::MDString::get(*irContext, identifier);
        specLib->setMetadata(PALLAS_SPEC_LIB_MARKER,
                             llvm::MDNode::get(*irContext, idStr));
    }

    llvm::Type *SpecUnifier::getBoolTy()
    {
        return llvm::Type::getInt1Ty(*irContext);
    }

    llvm::Type *SpecUnifier::getVoidTy()
    {
        return llvm::Type::getVoidTy(*irContext);
    }

    void SpecUnifier::addSret(
        llvm::CallInst *call, unsigned int argNo, llvm::Type *rType)
    {
        call->addParamAttr(
            argNo, llvm::Attribute::getWithStructRetType(*irContext, rType));
    }

    void SpecUnifier::addByVal(
        llvm::CallInst *call, unsigned int argNo, llvm::Type *type)
    {
        call->addParamAttr(
            argNo, llvm::Attribute::getWithByValType(*irContext, type));
    }

    void SpecUnifier::applyAttrToReturn(
        const IRTypeRepresentation &retRepr, llvm::Function *func)
    {
        if (retRepr.isReturnByParam())
        {
            for (auto a : retRepr.getAttributes(0))
                func->addParamAttr(0, a);
        }
        else
        {
            for (auto a : retRepr.getAttributes(0))
                func->addRetAttr(a);
        }
    }

    void SpecUnifier::applyAttrToParamRepr(const IRTypeRepresentation &pRepr,
                                           llvm::Function *func, unsigned int argIdx)
    {
        for (unsigned int i = 0; i < pRepr.getNumElems(); ++i)
            for (auto a : pRepr.getAttributes(i))
                func->addParamAttr(argIdx + i, a);
    }

    void SpecUnifier::applyAttrToReturn(const IRTypeRepresentation &retRepr,
                                        llvm::CallInst *call)
    {
        if (retRepr.isReturnByParam())
        {
            for (auto a : retRepr.getAttributes(0))
                call->addParamAttr(0, a);
        }
        else
        {
            for (auto a : retRepr.getAttributes(0))
                call->addRetAttr(a);
        }
    }

    void SpecUnifier::applyAttrToParamRepr(const IRTypeRepresentation &pRepr,
                                           llvm::CallInst *call, unsigned int argIdx)
    {
        for (unsigned int i = 0; i < pRepr.getNumElems(); ++i)
            for (auto a : pRepr.getAttributes(i))
                call->addParamAttr(argIdx + i, a);
    }
}