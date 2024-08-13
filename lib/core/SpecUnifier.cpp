#include "SpecUnifier.h"
#include "IrSpecConstants.h"
#include "MetadataUtils.h"
#include "PallasException.h"

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

    void SpecUnifier::unifySpecifications(llvm::Module *irModule, llvm::LLVMContext *irContext)
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
                throw utils::PallasException("Failed to remove specification function (" + srcSpecFunName + "). Remaining users exist.");
            }

            // Remove the function from the module
            srcSpecFun->eraseFromParent();
        }
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

            auto newCall = transformSpecCall(*oldCall);
            if (!newCall)
            {
                auto srcSpecName = oldCall->getCalledFunction()->getName().str();
                throw utils::PallasException("Failed to generate a new call to replace use of " + srcSpecName);
            }

            // Transfer Metadata
            /* TODO: Check if it makes sense to transfer the line/ column-info from the old call to the new call.
             * The metadata from the old call is not very helpfull, as it refers to the line- and column-numbers
             * within the generated temporary file. Hence we have to derive the approprate location information from
             * the parser.
             */

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
        case SeqAppend:
            return transformSeqAppend(oldCall);
        default:
            throw utils::PallasException("Encountered an unsupported type of specification-call.");
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

    llvm::Function *SpecUnifier::getUSpecOld(llvm::Type *type)
    {
        if (specOld.find(type) == specOld.end())
        {
            // Generate declaration for new old-function with the given type
            // TODO: Consider sret when returning structs.
            auto cnt = specOld.size();
            auto specName = PALLAS_SPEC_OLD_BASE + "." + std::to_string(cnt);
            llvm::SmallVector<llvm::Type *> specArgs(
                {llvm::PointerType::get(*irContext, 0)});
            auto specFType = llvm::FunctionType::get(type, specArgs, false);
            auto specFun = llvm::Function::Create(
                specFType, llvm::Function::ExternalLinkage, specName, irModule);
            specOld[type] = specFun;
            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specFun, PALLAS_SPEC_OLD_BASE);
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
            llvm::SmallVector<llvm::Type *> specArgs;
            auto specFType = llvm::FunctionType::get(type, specArgs, false);
            auto specFun = llvm::Function::Create(
                specFType, llvm::Function::ExternalLinkage, specName, irModule);
            specResult[type] = specFun;
            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specFun, PALLAS_SPEC_RESULT_BASE);
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
            llvm::SmallVector<llvm::Type *> specArgs({llvm::PointerType::get(*irContext, 0)});
            auto specFType = llvm::FunctionType::get(type, specArgs, false);
            auto specFun = llvm::Function::Create(
                specFType, llvm::Function::ExternalLinkage, specName, irModule);
            specBoundVar[type] = specFun;
            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specFun, PALLAS_SPEC_BOUND_VAR);
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
                llvm::PointerType::get(getUSpecFracType(), 0), // Opaque, or non-opaque pointer?
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

    llvm::Function *SpecUnifier::getUSpecSeqGet(llvm::Type *type)
    {
        if (specSeqGet.find(type) == specSeqGet.end())
        {
            // Generate function declaration
            auto cnt = specSeqGet.size();
            auto name = PALLAS_SPEC_SEQ_GET_BASE + "." + std::to_string(cnt);
            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(getUSpecSeqType(type), 0),
                llvm::Type::getInt64Ty(*irContext)};
            auto specFun = createSpecLibFunction(name, type, argList);
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specFun->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specFun->addParamAttr(1, llvm::Attribute::NoUndef);
            specSeqGet[type] = specFun;
        }

        return specSeqGet[type];
    }

    llvm::Function *SpecUnifier::getUSpecSeqAppend(llvm::Type *type)
    {
        if (specSeqAppend.find(type) == specSeqAppend.end())
        {
            // Generate function declaration
            auto cnt = specSeqAppend.size();
            auto name = PALLAS_SPEC_SEQ_APPEND_BASE + "." + std::to_string(cnt);
            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(getUSpecSeqType(type), 0),
                llvm::PointerType::get(getUSpecSeqType(type), 0),
                type};
            auto specFun = createSpecLibFunction(name, getVoidTy(), argList);
            specFun->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(type)));
            specFun->addParamAttr(1, llvm::Attribute::NoUndef);
            specFun->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specFun->addParamAttr(2, llvm::Attribute::NoUndef);
            specSeqAppend[type] = specFun;
        }

        return specSeqAppend[type];
    }

    llvm::Function *SpecUnifier::createSpecLibFunction(const std::string &name, llvm::Type *returnType, llvm::SmallVector<llvm::Type *> argTypes)
    {
        auto specFunType = llvm::FunctionType::get(returnType, argTypes, false);
        auto specFunction = llvm::Function::Create(specFunType, llvm::Function::ExternalLinkage, name, irModule);
        // Add metadata-marker to mark as specification-library-function
        markAsSpecLibFunction(specFunction, name);
        return specFunction;
    }

    void SpecUnifier::markAsSpecLibFunction(llvm::Function *specLib, const std::string &identifier)
    {
        auto idStr = llvm::MDString::get(*irContext, identifier);
        specLib->setMetadata(PALLAS_SPEC_LIB_MARKER, llvm::MDNode::get(*irContext, idStr));
    }

    llvm::Type *SpecUnifier::getBoolTy()
    {
        return llvm::Type::getInt1Ty(*irContext);
    }

    llvm::Type *SpecUnifier::getVoidTy()
    {
        return llvm::Type::getVoidTy(*irContext);
    }
}