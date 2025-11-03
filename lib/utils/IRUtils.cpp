#include "IRUtils.h"

#include <cassert>
#include <utility>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Metadata.h>
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

        // Adapt the type of the DISubprogram
        auto *oldSProg = wFunc->getSubprogram();
        auto *oldSProgType = wFunc->getSubprogram()->getType();
        llvm::DIBuilder diBuilder(*wFunc->getParent(), true, oldSProg->getUnit());

        llvm::SmallVector<llvm::Metadata *> newSProgTypeArr;
        for (auto [idx, t] : llvm::enumerate(oldSProgType->getTypeArray()))
        {
            if (idx == 0 ||                                     // Return-type
                !argsToRemove.contains(wFunc->getArg(idx - 1))) // Arg-types
            {
                newSProgTypeArr.push_back(t);
            }
        }
        auto *newSProgType = diBuilder.createSubroutineType(
            diBuilder.getOrCreateTypeArray(newSProgTypeArr),
            oldSProgType->getFlags(),
            oldSProgType->getCC());
        newFunc->getSubprogram()->replaceType(newSProgType);

        // Build new DISubprogram
        // TODO: Change type and remove declaration-flag
        //       or change the type of the declarartion
        /*
        auto *newSProg = diBuilder.createFunction(
            oldSProg->getScope(),
            oldSProg->getName(),
            oldSProg->getLinkageName(),
            oldSProg->getFile(),
            oldSProg->getLine(),
            newSProgType,
            oldSProg->getScopeLine(),
            oldSProg->getFlags(),
            oldSProg->getSPFlags(),
            oldSProg->getTemplateParams(),
            nullptr, // Set declaration to null
            oldSProg->getThrownTypes(),
            oldSProg->getAnnotations(),
            oldSProg->getTargetFuncName()
        );
        newFunc->setSubprogram(newSProg);
        //*/

        // If the subprog has a declaration, change type of that as well
        // Because it is usually not distinct, we cannot use replaceType
        // TODO: Cleanup all refrences to the old declaration-SProg
        // if (auto *diDecl = sProg->getDeclaration()) {

        // auto newDeclSProg = diDecl->clone();
        // newDeclSProg->replaceType(newSProgType);
        // diDecl->replaceAllUsesWith(
        //    llvm::MDNode::replaceWithPermanent(std::move(newDeclSProg)));
        // diDecl->replaceType(newSProgType);
        // }

        diBuilder.finalize();
        wFunc->eraseFromParent();
    }

    llvm::DISubprogram *getDeclSubprog(const llvm::Function &f)
    {
        if (auto *sProg = f.getSubprogram())
        {
            if (auto sProgDecl = sProg->getDeclaration())
                return sProgDecl;
            else
                return sProg;
        }
        return nullptr;
    }
}