#include "transform/SpecUnifier.h"
#include "utils/IrSpecConstants.h"
#include "utils/MetadataUtils.h"

#include <iostream>
#include <stdexcept>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

namespace sourcespec2ir
{

    void SpecUnifier::unifySpecifications(llvm::Module *irModule, llvm::LLVMContext *irContext)
    {
        // TODO: Move the arguments to the constructor
        this->irModule = irModule;
        this->irContext = irContext;

        // Replace all allocations that use a type from the source-specification-library
        replaceSpecTypes();

        // Collect all used specification-functions from module
        llvm::SmallVector<llvm::Function *, 32> srcSpecFuns;
        for (auto &f : irModule->getFunctionList())
        {
            if (isSrcSpecificationFunction(f))
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
                throw std::logic_error("Failed to remove specification function (" + srcSpecFunName + "). Remaining users exist.");
            }

            // Remove the function from the module
            srcSpecFun->eraseFromParent();
        }
    }

    bool SpecUnifier::isSrcSpecificationFunction(const llvm::Function &f)
    {
        // TODO: Implement this better (i.e. check for scope or sth like that)
        auto fName = f.getName().str();
        if (fName == C_SPEC_IMPLY ||
            fName == C_SPEC_EXISTS ||
            fName == C_SPEC_FORALL ||
            fName == C_SPEC_FORALL_SEP ||
            fName == C_SPEC_FRAC_OF ||
            fName == C_SPEC_PERM)
        {
            return true;
        }
        else if (f.getName().startswith(C_SPEC_OLD_BASE) ||
                 f.getName().startswith(C_SPEC_RESULT_BASE) ||
                 f.getName().startswith(C_SPEC_BOUND_VAR) ||
                 f.getName().startswith(C_SPEC_SEQ_NEW_BASE) ||
                 f.getName().startswith(C_SPEC_SEQ_SIZE_BASE) ||
                 f.getName().startswith(C_SPEC_SEQ_GET_BASE) ||
                 f.getName().startswith(C_SPEC_SEQ_APPEND_BASE))
        {
            return true;
        }
        return false;
    }

    llvm::Type *SpecUnifier::getContentTypeOfADT(const llvm::Value &v)
    {
        // Get the content-type of the ADT from the first element of the struct
        // Currently, only types allocated via alloca are supported.
        // TODO: Expand this (arguments, ...)
        if (auto *allocInstr = llvm::dyn_cast<llvm::AllocaInst>(&v))
        {
            return getContentTypeOfADT(*allocInstr->getAllocatedType());
        }
        throw std::runtime_error("Failed to extract the content-type of the sequence-type.");
    }

    llvm::Type *SpecUnifier::getContentTypeOfADT(const llvm::Type &t)
    {
        if (auto *adtType = llvm::dyn_cast<llvm::StructType>(&t))
        {
            return adtType->getElementType(0);
        }
        throw std::runtime_error("Failed to extract the content-type of the sequence-type.");
    }

    void SpecUnifier::replaceCallsToSpecFunction(llvm::Function &srcSpecFunction)
    {
        while (!srcSpecFunction.user_empty())
        // for (auto user : srcSpecFunction.users())
        {
            llvm::User *user = *srcSpecFunction.user_begin();
            std::cerr << "===============================================" << std::endl;
            std::cerr << "Detected user of " << srcSpecFunction.getName().str() << ":" << std::endl;
            user->dump();
            std::cerr << "===============================================" << std::endl;

            auto oldCall = llvm::dyn_cast<llvm::CallInst>(user);
            if (!oldCall)
            {
                // Not a function call
                throw std::logic_error("Encountered a use of a specification-function that is not a call.");
            }

            auto newCall = createSpecCall(*oldCall);

            if (!newCall)
            {
                auto srcSpecName = oldCall->getCalledFunction()->getName().str();
                throw std::logic_error("Failed to generate a new call to replace use of " + srcSpecName);
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

    void SpecUnifier::replaceSpecTypes()
    {
        auto *srcFracType = getSourceLibFracType();

        // Collect all of the sequence-types that are used.
        llvm::SmallPtrSet<llvm::StructType *, 8> seqTypes;
        for (auto st : irModule->getIdentifiedStructTypes())
        {
            if (isSrcSeqType(*st))
            {
                seqTypes.insert(st);
            }
        }

        // If the type for fractions is not used, we do not need to replace anything.
        if (srcFracType == nullptr && seqTypes.empty())
        {
            std::cout << "Skipping replacement of spec-types." << std::endl;
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
                        // allocInstr->setAllocatedType(getUSpecFracType());
                    }
                    else if (auto *sType = llvm::dyn_cast<llvm::StructType>(oldType))
                    {
                        // Handle sequence-types
                        if (seqTypes.contains(sType))
                        {
                            auto contentType = getContentTypeOfADT(*sType);
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

    bool SpecUnifier::isSrcSeqType(const llvm::StructType &t)
    {
        return t.getName().startswith("struct." + C_SPEC_SEQ_TYPE_BASE);
    }

    llvm::Type *SpecUnifier::getSourceLibFracType()
    {
        return llvm::StructType::getTypeByName(*irContext, C_SPEC_FRAC_TYPE);
    }

    llvm::CallInst *SpecUnifier::createSpecCall(llvm::CallInst &oldCall)
    {
        auto srcSpecName = oldCall.getCalledFunction()->getName().str();
        if (srcSpecName == C_SPEC_IMPLY)
        {
            // Imply
            // Build new call
            // TODO: Check if we need to copy the Values here, or if taking the pointer is fine
            llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
            // If we had to do some argument translation, this would happen here
            auto newCall = llvm::CallInst::Create(getImplySpec(), newArgs);
            return newCall;
        }
        if (srcSpecName == C_SPEC_EXISTS)
        {
            // exists
            llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
            return llvm::CallInst::Create(getExistsSpec(), newArgs);
        }
        if (srcSpecName == C_SPEC_FORALL)
        {
            // forall
            llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
            return llvm::CallInst::Create(getForallSpec(), newArgs);
        }
        if (srcSpecName == C_SPEC_FORALL_SEP)
        {
            // forall*
            llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
            return llvm::CallInst::Create(getForallSepSpec(), newArgs);
        }
        if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_OLD_BASE))
        {
            // \old
            auto type = oldCall.getCalledFunction()->getReturnType();
            llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
            auto newCall = llvm::CallInst::Create(getOldSpec(type), newArgs);
            return newCall;
        }
        if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_RESULT_BASE))
        {
            // \result
            auto type = oldCall.getCalledFunction()->getReturnType();
            llvm::SmallVector<llvm::Value *, 1> newArgs;
            auto newCall = llvm::CallInst::Create(getResultSpec(type), newArgs);
            return newCall;
        }
        if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_BOUND_VAR))
        {
            // Bound variable
            auto type = oldCall.getCalledFunction()->getReturnType();
            llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
            auto newCall = llvm::CallInst::Create(getBoundVarSpec(type), newArgs);
            return newCall;
        }
        if (srcSpecName == C_SPEC_FRAC_OF)
        {
            // fracOf
            llvm::SmallVector<llvm::Value *, 4> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1),
                oldCall.getArgOperand(2)};
            auto newCall = llvm::CallInst::Create(getFracOfSpec(), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecFracType()));
            newCall->addParamAttr(1, llvm::Attribute::NoUndef);
            newCall->addParamAttr(2, llvm::Attribute::NoUndef);

            // Check that the type for fractions has been properly replaced.
            if (auto *allocInstr = llvm::dyn_cast<llvm::AllocaInst>(oldCall.getArgOperand(0)))
            {
                if (allocInstr->getAllocatedType() != getUSpecFracType())
                {
                    throw std::runtime_error("The spec-lib type for fractions has not been properly replaced.");
                }
            }
            else
            {
                throw std::runtime_error("Allocation for fractions is only supported using alloca.");
            }

            return newCall;
        }
        if (srcSpecName == C_SPEC_PERM)
        {
            // Perm
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1)};
            auto newCall = llvm::CallInst::Create(getPermSpec(), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::NoUndef);
            newCall->addParamAttr(1, llvm::Attribute::NoUndef);
            newCall->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, getUSpecFracType()));
            return newCall;
        }
        if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_SEQ_NEW_BASE))
        {
            // New sequence
            auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
            llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
            auto newCall = llvm::CallInst::Create(getSeqNewSpec(cType), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(cType)));
            return newCall;
        }
        if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_SEQ_SIZE_BASE))
        {
            // Size of sequences
            auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
            llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
            auto newCall = llvm::CallInst::Create(getSeqSizeSpec(cType), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::NoUndef);
            newCall->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(cType)));
            return newCall;
        }
        if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_SEQ_GET_BASE))
        {
            // Get on sequences
            auto cType = getContentTypeOfADT(*oldCall.getArgOperand(0));
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1),
            };
            auto newCall = llvm::CallInst::Create(getSeqGetSpec(cType), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::NoUndef);
            newCall->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(cType)));
            newCall->addParamAttr(1, llvm::Attribute::NoUndef);
            return newCall;
        }
        if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_SEQ_APPEND_BASE))
        {
            // Append on sequences
            auto cType = getContentTypeOfADT(*oldCall.getArgOperand(1));
            auto seqType = getUSpecSeqType(cType);
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(1),
                oldCall.getArgOperand(2),
            };
            auto newCall = llvm::CallInst::Create(getSeqAppendSpec(cType), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, seqType));
            newCall->addParamAttr(1, llvm::Attribute::NoUndef);
            newCall->addParamAttr(1, llvm::Attribute::getWithByValType(*irContext, seqType));
            newCall->addParamAttr(2, llvm::Attribute::NoUndef);
            return newCall;
        }

        throw std::runtime_error("Unification of " + srcSpecName + " is not yet supported!");
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
            auto tName = PALLAS_SPEC_TYPE_SEQ_BASE + "." + std::to_string(specSeqTypeCounter++);
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

    llvm::Function *SpecUnifier::getImplySpec()
    {
        if (!specImply)
        {
            // Generate and add declaration of imply-function
            // TODO: Check if zeroext, noundef attributes should be added
            llvm::SmallVector<llvm::Type *> implySpecArgs({llvm::Type::getInt1Ty(*irContext), llvm::Type::getInt1Ty(*irContext)});
            auto implySpecFT = llvm::FunctionType::get(llvm::Type::getInt1Ty(*irContext), implySpecArgs, false);
            specImply = llvm::Function::Create(implySpecFT, llvm::Function::ExternalLinkage, PALLAS_SPEC_IMPLY, irModule);

            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specImply, PALLAS_SPEC_IMPLY);
        }
        return specImply;
    }

    llvm::Function *SpecUnifier::getOldSpec(llvm::Type *type)
    {
        if (specOld.find(type) == specOld.end())
        {
            // Generate declaration for new old-function with the given type
            auto specName = PALLAS_SPEC_OLD_BASE + "." + std::to_string(specOldCounter++);
            llvm::SmallVector<llvm::Type *> specArgs({type});
            auto specFType = llvm::FunctionType::get(type, specArgs, false);
            auto specFun = llvm::Function::Create(specFType, llvm::Function::ExternalLinkage, specName, irModule);
            specOld[type] = specFun;
            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specFun, PALLAS_SPEC_OLD_BASE);
        }

        return specOld[type];
    }

    llvm::Function *SpecUnifier::getResultSpec(llvm::Type *type)
    {
        if (specResult.find(type) == specResult.end())
        {
            // Generate declaration for new result-function with the given type
            auto specName = PALLAS_SPEC_RESULT_BASE + "." + std::to_string(specResultCounter++);
            llvm::SmallVector<llvm::Type *> specArgs;
            auto specFType = llvm::FunctionType::get(type, specArgs, false);
            auto specFun = llvm::Function::Create(specFType, llvm::Function::ExternalLinkage, specName, irModule);
            specResult[type] = specFun;
            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specFun, PALLAS_SPEC_RESULT_BASE);
        }

        return specResult[type];
    }

    llvm::Function *SpecUnifier::getBoundVarSpec(llvm::Type *type)
    {
        if (specBoundVar.find(type) == specBoundVar.end())
        {
            // Generate declaration
            auto specName = PALLAS_SPEC_BOUND_VAR + "." + std::to_string(specBoundVarCounter++);
            llvm::SmallVector<llvm::Type *> specArgs({llvm::PointerType::get(*irContext, 0)});
            auto specFType = llvm::FunctionType::get(type, specArgs, false);
            auto specFun = llvm::Function::Create(specFType, llvm::Function::ExternalLinkage, specName, irModule);
            specBoundVar[type] = specFun;
            // Add metadata-marker to mark as specification-library-function
            markAsSpecLibFunction(specFun, PALLAS_SPEC_BOUND_VAR);
        }

        return specBoundVar[type];
    }

    llvm::Function *SpecUnifier::getExistsSpec()
    {
        if (!specExists)
        {
            specExists = createSpecLibFunction(PALLAS_SPEC_EXISTS,
                                               getBoolTy(), {getBoolTy(), getBoolTy()});
        }
        return specExists;
    }

    llvm::Function *SpecUnifier::getForallSpec()
    {
        if (!specForall)
        {
            specForall = createSpecLibFunction(PALLAS_SPEC_FORALL,
                                               getBoolTy(), {getBoolTy(), getBoolTy()});
        }
        return specForall;
    }

    llvm::Function *SpecUnifier::getForallSepSpec()
    {
        if (!specForallSep)
        {
            specForallSep = createSpecLibFunction(PALLAS_SPEC_FORALL_SEP,
                                                  getBoolTy(), {getBoolTy(), getBoolTy()});
        }
        return specForallSep;
    }

    llvm::Function *SpecUnifier::getFracOfSpec()
    {
        if (!specFracOf)
        {

            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(getUSpecFracType(), 0), // Opaque, or non-opaque pointer?
                llvm::Type::getInt32Ty(*irContext),            // TODO: Change to systems default integer
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

    llvm::Function *SpecUnifier::getPermSpec()
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

    llvm::Function *SpecUnifier::getSeqNewSpec(llvm::Type *type)
    {
        if (specSeqNew.find(type) == specSeqNew.end())
        {
            // Generate function declaration
            auto specName = PALLAS_SPEC_SEQ_NEW_BASE + "." + std::to_string(specSeqNewCounter++);
            auto retType = llvm::PointerType::get(getUSpecSeqType(type), 0);
            auto specFun = createSpecLibFunction(specName, getVoidTy(), {retType});
            specFun->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, getUSpecSeqType(type)));
            specSeqNew[type] = specFun;
        }

        return specSeqNew[type];
    }

    llvm::Function *SpecUnifier::getSeqSizeSpec(llvm::Type *type)
    {
        if (specSeqSize.find(type) == specSeqSize.end())
        {
            // Generate function declaration
            auto name = PALLAS_SPEC_SEQ_SIZE_BASE + "." + std::to_string(specSeqSizeCounter++);
            auto argType = llvm::PointerType::get(getUSpecSeqType(type), 0);
            auto specFun = createSpecLibFunction(name, llvm::Type::getInt64Ty(*irContext), {argType});
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specFun->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specSeqSize[type] = specFun;
        }

        return specSeqSize[type];
    }

    llvm::Function *SpecUnifier::getSeqGetSpec(llvm::Type *type)
    {
        if (specSeqGet.find(type) == specSeqGet.end())
        {
            // Generate function declaration
            auto name = PALLAS_SPEC_SEQ_GET_BASE + "." + std::to_string(specSeqGetCounter++);
            llvm::SmallVector<llvm::Type *> argList = {
                llvm::PointerType::get(getUSpecSeqType(type), 0),
                llvm::Type::getInt64Ty(*irContext) // TODO: Remove the hardcoded 64-bit-type
            };
            auto specFun = createSpecLibFunction(name, type, argList);
            specFun->addParamAttr(0, llvm::Attribute::NoUndef);
            specFun->addParamAttr(0, llvm::Attribute::getWithByValType(*irContext, getUSpecSeqType(type)));
            specFun->addParamAttr(1, llvm::Attribute::NoUndef);
            specSeqGet[type] = specFun;
        }

        return specSeqGet[type];
    }

    llvm::Function *SpecUnifier::getSeqAppendSpec(llvm::Type *type)
    {
        if (specSeqAppend.find(type) == specSeqAppend.end())
        {
            // Generate function declaration
            auto name = PALLAS_SPEC_SEQ_APPEND_BASE + "." + std::to_string(specSeqAppendCounter++);
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