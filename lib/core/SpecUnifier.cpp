#include "SpecUnifier.h"
#include "IrSpecConstants.h"
#include "MetadataUtils.h"
#include "PallasException.h"
#include "IrLookup.h"

#include <iostream>
#include <stdexcept>
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

    llvm::CallInst *SpecUnifier::transformSepConj(llvm::CallInst &oldCall)
    {
        llvm::SmallVector<llvm::Value *, 2> newArgs = {
            oldCall.getArgOperand(0),
            oldCall.getArgOperand(1)};
        return llvm::CallInst::Create(getUSpecSepConj(), newArgs);
    }

    llvm::CallInst *SpecUnifier::transformOld(llvm::CallInst &oldCall)
    {
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            // Struct-type
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1)};
            auto newCall = llvm::CallInst::Create(getUSpecOld(type), newArgs);
            addSret(newCall, 0, type);
            addByVal(newCall, 1, type);
            return newCall;
        }
        else
        {
            return llvm::CallInst::Create(getUSpecOld(type),
                                          {oldCall.getArgOperand(0)});
        }
    }

    llvm::CallInst *SpecUnifier::transformResult(llvm::CallInst &oldCall)
    {
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            auto newCall = llvm::CallInst::Create(
                getUSpecResult(type), {oldCall.getArgOperand(0)});
            addSret(newCall, 0, type);
            return newCall;
        }
        else
        {
            llvm::SmallVector<llvm::Value *, 1> newArgs;
            return llvm::CallInst::Create(getUSpecResult(type), newArgs);
        }
    }

    llvm::CallInst *SpecUnifier::transformBoundVar(llvm::CallInst &oldCall)
    {
        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        if (llvm::isa<llvm::StructType>(type))
        {
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1)};
            auto newCall = llvm::CallInst::Create(getUSpecBoundVar(type), newArgs);
            addSret(newCall, 0, type);
            return newCall;
        }
        else
        {
            return llvm::CallInst::Create(
                getUSpecBoundVar(type), {oldCall.getArgOperand(0)});
        }
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

        // Check if the result is returned as an sret-parameter
        auto isParamReturn = oldCall.getCalledFunction()->getReturnType()->isVoidTy();

        // Get the content-type of the sequence from the argument
        auto cType = getContentTypeOfADT(
            *oldCall.getArgOperand(isParamReturn ? 1 : 0));

        // Build type-representation of the return-type
        IRTypeRepresentation retTRepr;
        auto retAttrs = isParamReturn ? oldCall.getAttributes().getParamAttrs(0)
                                      : oldCall.getAttributes().getRetAttrs();
        retTRepr.addType(oldCall.getCalledFunction()->getReturnType(), retAttrs);

        // Build new arg-list by copying arguments 1:1
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < numArgs; ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        // Build the new call
        auto newCall = llvm::CallInst::Create(
            getUSpecSeqGet(cType, retTRepr), newArgs);

        // Add attributes:
        if (isParamReturn) 
            for (auto a : retTRepr.getAttributes(0))
                newCall->addParamAttr(0, a); 

        newCall->addParamAttr(isParamReturn ? 1 : 0, llvm::Attribute::NoUndef);
        addByVal(newCall, isParamReturn ? 1 : 0, getUSpecSeqType(cType));
        newCall->addParamAttr(isParamReturn ? 2 : 1, llvm::Attribute::NoUndef);

        if (!isParamReturn)
            for (auto a : retTRepr.getAttributes(0))
                newCall->addRetAttr(a); 

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
        auto numArgs = oldCall.getNumOperands() - 1;

        // Obtain the content-type from the sequence that is passed as the first arg.
        auto cType = getContentTypeOfADT(*oldCall.getArgOperand(numArgs - 1));
        auto seqType = getUSpecSeqType(cType);

        // Copy types and arguments of the passed element's representation.
        IRTypeRepresentation elemTRepr;
        for (unsigned int i = 1; i < numArgs - 1; ++i)
        {
            auto arg = oldCall.getArgOperand(i);
            auto argT = arg->getType();
            auto argAttrs = oldCall.getAttributes().getParamAttrs(i);
            elemTRepr.addType(argT, argAttrs);
        }

        // Copy arguments 1:1 to the new call
        llvm::SmallVector<llvm::Value *> newArgs;
        for (unsigned int i = 0; i < numArgs; ++i)
            newArgs.push_back(oldCall.getArgOperand(i));

        auto newCall = llvm::CallInst::Create(
            getUSpecSeqPrepend(cType, elemTRepr), newArgs);

        // Add attributes to return-type
        addSret(newCall, 0, seqType);
        // Add Attributes to representation of the prepended element
        for (unsigned int i = 0; i < elemTRepr.getNumElems(); ++i)
            for (auto attr : elemTRepr.getAttributes(i))
                newCall->addParamAttr(i + 1, attr);
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

    llvm::Type *SpecUnifier::getContentTypeOfADT(const llvm::Value &v)
    {
        // Get the content-type of the ADT from the first element of the struct
        // Currently, only types allocated via alloca are supported.
        // TODO: Expand this (arguments, ...)
        if (auto *allocInstr = llvm::dyn_cast<llvm::AllocaInst>(&v))
        {
            return getContentTypeOfADTTy(*allocInstr->getAllocatedType());
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
            if (!utils::isGeneratedSpecFunction(*parentF))
            {
                throw utils::PallasException(
                    "Encountered a call to a specification-function from a non-wrapper function.");
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
            if (!utils::isGeneratedSpecFunction(f))
                continue;

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

    llvm::StructType *SpecUnifier::getUSpecSeqType(llvm::Type *type)
    {
        if (specSeqTypes.find(type) == specSeqTypes.end())
        {
            // Generate declaration for new sequence-type
            auto cnt = specSeqTypes.size();
            auto tName = PALLAS_SPEC_TYPE_SEQ_BASE + "." + std::to_string(cnt);
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

    llvm::Function *SpecUnifier::getUSpecSepConj()
    {
        if (!specSepConj)
        {
            specSepConj = createSpecLibFunction(
                PALLAS_SPEC_SEPCONJ, getBoolTy(), {getBoolTy(), getBoolTy()});
        }
        return specSepConj;
    }

    llvm::Function *SpecUnifier::getUSpecOld(llvm::Type *type)
    {
        if (specOld.find(type) == specOld.end())
        {
            // Generate declaration for new old-function with the given type
            auto cnt = specOld.size();
            auto specName = PALLAS_SPEC_OLD_BASE + "." + std::to_string(cnt);

            // Function-type differs for struct-types and non-struct-types.
            llvm::Function *specFun = nullptr;
            if (llvm::isa<llvm::StructType>(type))
            {
                // struct-type
                llvm::SmallVector<llvm::Type *> specArgs(
                    {llvm::PointerType::get(*irContext, 0),
                     llvm::PointerType::get(*irContext, 0)});
                specFun = createSpecLibFunction(specName, getVoidTy(), specArgs,
                                                PALLAS_SPEC_OLD_BASE);
                specFun->addParamAttr(0, llvm::Attribute::getWithStructRetType(
                                             *irContext, type));
                specFun->addParamAttr(1, llvm::Attribute::getWithByValType(
                                             *irContext, type));
            }
            else
            {
                // Non-struct-type
                llvm::SmallVector<llvm::Type *> specArgs({type});
                specFun = createSpecLibFunction(specName, type, specArgs,
                                                PALLAS_SPEC_OLD_BASE);
            }
            specOld[type] = specFun;
        }

        return specOld[type];
    }

    llvm::Function *SpecUnifier::getUSpecResult(llvm::Type *type)
    {
        if (specResult.find(type) == specResult.end())
        {
            // Generate declaration for new result-function with the given type
            auto cnt = specResult.size();
            auto specName = PALLAS_SPEC_RESULT_BASE + "." + std::to_string(cnt);

            // Function-types differ for struct- and non-struct-types
            if (llvm::isa<llvm::StructType>(type))
            {
                // struct type
                auto specFun = createSpecLibFunction(
                    specName, getVoidTy(),
                    {llvm::PointerType::get(*irContext, 0)},
                    PALLAS_SPEC_RESULT_BASE);
                specFun->addParamAttr(0, llvm::Attribute::getWithStructRetType(
                                             *irContext, type));
                specResult[type] = specFun;
            }
            else
            {
                // non-struct-type
                specResult[type] = createSpecLibFunction(
                    specName, type, {}, PALLAS_SPEC_RESULT_BASE);
            }
        }

        return specResult[type];
    }

    llvm::Function *SpecUnifier::getUSpecBoundVar(llvm::Type *type)
    {
        if (specBoundVar.find(type) == specBoundVar.end())
        {
            // Generate declaration
            auto cnt = specBoundVar.size();
            auto specName = PALLAS_SPEC_BOUND_VAR + "." + std::to_string(cnt);

            if (llvm::isa<llvm::StructType>(type))
            {
                // struct type
                llvm::SmallVector<llvm::Type *> args(
                    {llvm::PointerType::get(*irContext, 0),
                     llvm::PointerType::get(*irContext, 0)});
                auto specFun = createSpecLibFunction(
                    specName, getVoidTy(), args, PALLAS_SPEC_BOUND_VAR);
                specFun->addParamAttr(0, llvm::Attribute::getWithStructRetType(
                                             *irContext, type));
                specBoundVar[type] = specFun;
            }
            else
            {
                // non-struct-type
                llvm::SmallVector<llvm::Type *> args(
                    {llvm::PointerType::get(*irContext, 0)});
                specBoundVar[type] = createSpecLibFunction(
                    specName, type, args, PALLAS_SPEC_BOUND_VAR);
            }
        }

        return specBoundVar[type];
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

    llvm::Function *SpecUnifier::getUSpecSeqNew(llvm::Type *type)
    {
        if (specSeqNew.find(type) == specSeqNew.end())
        {
            // Generate function declaration
            auto cnt = specSeqNew.size();
            auto specName = PALLAS_SPEC_SEQ_NEW_BASE + "." + std::to_string(cnt);
            auto retType = llvm::PointerType::get(getUSpecSeqType(type), 0);
            auto specFun = createSpecLibFunction(specName, getVoidTy(), {retType});
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
            auto cnt = specSeqSize.size();
            auto name = PALLAS_SPEC_SEQ_SIZE_BASE + "." + std::to_string(cnt);
            auto argType = llvm::PointerType::get(getUSpecSeqType(type), 0);
            auto specFun = createSpecLibFunction(name, llvm::Type::getInt64Ty(*irContext), {argType});
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specFun->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specSeqSize[type] = specFun;
        }

        return specSeqSize[type];
    }

    llvm::Function *SpecUnifier::getUSpecSeqGet(llvm::Type *type,
                                                const IRTypeRepresentation &typeRepr)
    {
        if (specSeqGet.find(type) == specSeqGet.end())
        {
            if (typeRepr.getTypes().size() != 1)
                throw utils::PallasException("Received more than one return-type");

            auto cnt = specSeqGet.size();
            auto name = PALLAS_SPEC_SEQ_GET_BASE + "." + std::to_string(cnt);

            // Check if the result is returned directly, or as a sret-parameter.
            auto isParamReturn = typeRepr.isReturnByParam();
            auto retT = isParamReturn ? getVoidTy()
                                      : typeRepr.getTypes().front();

            // Build argument-list
            llvm::SmallVector<llvm::Type *> argList;
            if (isParamReturn)
                argList.push_back(llvm::PointerType::get(*irContext, 0));
            argList.push_back(llvm::PointerType::get(getUSpecSeqType(type), 0));
            argList.push_back(llvm::Type::getInt64Ty(*irContext));
            // Build new function
            auto specFun = createSpecLibFunction(
                name, retT, argList, PALLAS_SPEC_SEQ_GET_BASE);
            // Add attributes
            if (isParamReturn)
                for (auto a : typeRepr.getAttributes(0))
                    specFun->addParamAttr(0, a);

            specFun->addParamAttr(isParamReturn ? 1 : 0, llvm::Attribute::NoUndef);
            specFun->addParamAttr(isParamReturn ? 1 : 0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specFun->addParamAttr(isParamReturn ? 2 : 1, llvm::Attribute::NoUndef);

            if (!isParamReturn)
                for (auto a : typeRepr.getAttributes(0))
                    specFun->addRetAttr(a);

            specSeqGet[type] = specFun;
        }

        return specSeqGet[type];
    }

    llvm::Function *SpecUnifier::getUSpecSeqPrepend(
        llvm::Type *type, const IRTypeRepresentation &typeRepr)
    {
        if (specSeqPrepend.find(type) == specSeqPrepend.end())
        {
            auto cnt = specSeqPrepend.size();
            auto name = PALLAS_SPEC_SEQ_PREPEND_BASE + "." + std::to_string(cnt);
            // const bool isStructT = llvm::isa<llvm::StructType>(type);

            llvm::SmallVector<llvm::Type *> argList;
            // Pointer for the return-value
            argList.push_back(llvm::PointerType::get(*irContext, 0));
            // Representation of the element that is prepended
            for (auto t : typeRepr.getTypes())
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
            unsigned int idx = 0; // Index error !!!!!
            while (idx < typeRepr.getNumElems())
            {
                for (auto a : typeRepr.getAttributes(idx))
                    specFun->addParamAttr(idx + 1, a);
                ++idx;
            }

            // Passed-in sequence
            specFun->addParamAttr(argList.size() - 1, llvm::Attribute::NoUndef);
            specFun->addParamAttr(
                argList.size() - 1,
                llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specSeqPrepend[type] = specFun;
        }

        return specSeqPrepend[type];
    }

    llvm::Function *SpecUnifier::getUSpecSeqEquals(llvm::Type *type)
    {
        if (specSeqEquals.find(type) == specSeqEquals.end())
        {
            auto cnt = specSeqEquals.size();
            auto name = PALLAS_SPEC_SEQ_EQUALS_BASE + "." + std::to_string(cnt);

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
}