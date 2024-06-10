#include "transform/SpecUnifier.h"
#include "utils/IrSpecConstants.h"

#include <iostream>
#include <stdexcept>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

namespace sourcespec2ir
{

    void SpecUnifier::unifySpecifications(llvm::Module *irModule, llvm::LLVMContext *irContext)
    {
        // TODO: Move the arguments to the constructor
        this->irModule = irModule;
        this->irContext = irContext;

        // TODO: Generate the required type-declarations

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
            fName == C_SPEC_FORALL_SEP)
        {
            return true;
        }
        else if (f.getName().startswith(C_SPEC_OLD_BASE) ||
                 f.getName().startswith(C_SPEC_RESULT_BASE) ||
                 f.getName().startswith(C_SPEC_BOUND_VAR))
        {
            return true;
        }
        return false;
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

    /**
     * TODO: Merge this with isSrcSpecificationFunction by returning a pointer instead of a bool.
     */
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
        else if (srcSpecName == C_SPEC_EXISTS)
        {
            // exists
            llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
            return llvm::CallInst::Create(getExistsSpec(), newArgs);
        }
        else if (srcSpecName == C_SPEC_FORALL)
        {
            // forall
            llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
            return llvm::CallInst::Create(getForallSpec(), newArgs);
        }
        else if (srcSpecName == C_SPEC_FORALL_SEP)
        {
            // forall*
            llvm::SmallVector<llvm::Value *, 2> newArgs = {oldCall.getArgOperand(0), oldCall.getArgOperand(1)};
            return llvm::CallInst::Create(getForallSepSpec(), newArgs);
        }
        else if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_OLD_BASE))
        {
            // \old
            auto type = oldCall.getCalledFunction()->getReturnType();
            llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
            auto newCall = llvm::CallInst::Create(getOldSpec(type), newArgs);
            return newCall;
        }
        else if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_RESULT_BASE))
        {
            // \result
            auto type = oldCall.getCalledFunction()->getReturnType();
            llvm::SmallVector<llvm::Value *, 1> newArgs;
            auto newCall = llvm::CallInst::Create(getResultSpec(type), newArgs);
            return newCall;
        }
        else if (oldCall.getCalledFunction()->getName().startswith(C_SPEC_BOUND_VAR))
        {
            // Bound variable
            auto type = oldCall.getCalledFunction()->getReturnType();
            llvm::SmallVector<llvm::Value *, 1> newArgs = {oldCall.getArgOperand(0)};
            auto newCall = llvm::CallInst::Create(getBoundVarSpec(type), newArgs);
            return newCall;
        }
        else
        {
            throw std::runtime_error("Unification of " + srcSpecName + " is not yet supported!");
        }
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
            specExists = createSpecLibFunction(PALLAS_SPEC_EXISTS, llvm::Type::getInt1Ty(*irContext),
                                               {llvm::Type::getInt1Ty(*irContext), llvm::Type::getInt1Ty(*irContext)});
        }
        return specExists;
    }

    llvm::Function *SpecUnifier::getForallSpec()
    {
        if (!specForall)
        {
            specForall = createSpecLibFunction(PALLAS_SPEC_FORALL, llvm::Type::getInt1Ty(*irContext),
                                               {llvm::Type::getInt1Ty(*irContext), llvm::Type::getInt1Ty(*irContext)});
        }
        return specForall;
    }

    llvm::Function *SpecUnifier::getForallSepSpec()
    {
        if (!specForallSep)
        {
            specForallSep = createSpecLibFunction(PALLAS_SPEC_FORALL_SEP, llvm::Type::getInt1Ty(*irContext),
                                                  {llvm::Type::getInt1Ty(*irContext), llvm::Type::getInt1Ty(*irContext)});
        }
        return specForallSep;
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
}