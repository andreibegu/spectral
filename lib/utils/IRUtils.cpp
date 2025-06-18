#include "IRUtils.h"

#include <cassert>
#include <utility>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>

namespace sourcespec2ir::utils
{
    void removeArgsFromWrapper(
        llvm::Function *wFunc,
        llvm::SmallSet<llvm::Argument *, 2> &argsToRemove)
    {
        // Based on LLVM's DeadArgumentElimination-Pass
        // https://github.com/llvm/llvm-project/blob/main/llvm/lib/Transforms/IPO/DeadArgumentElimination.cpp#L728
        if (wFunc == nullptr || argsToRemove.empty())
            return;

        // Build the type of the new function:
        // I.e. copy the types and attributes of all remaining arguments
        auto &ctx = wFunc->getContext();
        auto *oldFType = wFunc->getFunctionType();
        const auto &oldAttrs = wFunc->getAttributes();
        llvm::SmallVector<llvm::Type *> newParams;
        llvm::SmallVector<llvm::AttributeSet> newParamAttrs;

        for (auto [argIdx, arg] : llvm::enumerate(wFunc->args()))
        {
            if (!argsToRemove.contains(&arg))
            {
                newParams.push_back(arg.getType());
                newParamAttrs.push_back(oldAttrs.getParamAttrs(argIdx));
            }
        }
        llvm::AttributeSet newRetAttrs = oldAttrs.getRetAttrs();
        llvm::AttributeSet newFuncAttrs = oldAttrs.getFnAttrs().removeAttribute(
            ctx, llvm::Attribute::AllocSize);

        auto newAttrs = llvm::AttributeList::get(
            ctx, newFuncAttrs, newRetAttrs, newParamAttrs);
        auto *newFType = llvm::FunctionType::get(
            oldFType->getReturnType(), newParams, oldFType->isVarArg());

        // Build the new function
        auto *newFunc = llvm::Function::Create(
            newFType, wFunc->getLinkage(), wFunc->getAddressSpace());
        newFunc->copyAttributesFrom(wFunc);
        newFunc->setComdat(wFunc->getComdat());
        newFunc->setAttributes(newAttrs);
        wFunc->getParent()->getFunctionList().insert(wFunc->getIterator(), newFunc);
        newFunc->takeName(wFunc);
        // TODO: Add this once we move to a newer LLVM-version
        // newFunc->IsNewDbgInfoFormat = wFunc->IsNewDbgInfoFormat;

        // Move function-body to new Function
        newFunc->splice(newFunc->begin(), wFunc);

        for (auto itOld = wFunc->arg_begin(), end = wFunc->arg_end(),
                  itNew = newFunc->arg_begin();
             itOld != end; ++itOld)
        {
            if (!argsToRemove.contains(&*itOld))
            {
                (*itOld).replaceAllUsesWith(&*itNew);
                itNew->takeName(&*itOld);
                ++itNew;
            }
            else
            {
                // TODO: Do we need this?
                itOld->replaceAllUsesWith(llvm::PoisonValue::get(itOld->getType()));
            }
        }

        // Copy over Debug-info
        llvm::SmallVector<std::pair<unsigned, llvm::MDNode *>, 1> oldMD;
        wFunc->getAllMetadata(oldMD);
        for (auto [KindID, Node] : oldMD)
            newFunc->addMetadata(KindID, *Node);
        
        // TODO: Make sure that the DISubprogram debug-info is adapted!
        auto *sProg = newFunc->getSubprogram();


        wFunc->eraseFromParent();
    }
}