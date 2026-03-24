#include "MetadataUtils.h"
#include "IrSpecConstants.h"

#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Instructions.h>

namespace sourcespec2ir
{
    namespace utils
    {

        void markGeneratedSpecFunction(llvm::Function &f, llvm::LLVMContext &context)
        {
            auto idStr = llvm::MDString::get(context, "");
            f.setMetadata(PALLAS_GENERATED_MARKER, llvm::MDNode::get(context, idStr));
        }

        bool isGeneratedSpecFunction(const llvm::Function &f)
        {
            return f.getMetadata(PALLAS_GENERATED_MARKER) != nullptr;
        }

        void markGhostWrapper(llvm::Function &f, llvm::LLVMContext &context)
        {
            auto idStr = llvm::MDString::get(context, "");
            f.setMetadata(PALLAS_GHOST_WRAPPER_MARKER, llvm::MDNode::get(context, idStr));
        }

        bool isGhostWrapper(const llvm::Function &f)
        {
            return f.getMetadata(PALLAS_GHOST_WRAPPER_MARKER) != nullptr;
        }

        void markPredicateDef(llvm::Function &f, llvm::LLVMContext &context,
                              bool isInline)
        {
            auto inlineFlag = llvm::ConstantAsMetadata::get(
                llvm::ConstantInt::getBool(context, isInline));
            f.setMetadata(PALLAS_PREDICATE_DEF_MARKER,
                          llvm::MDNode::get(context, inlineFlag));
        }

        bool isPredicateDef(const llvm::Function &f)
        {
            return f.getMetadata(PALLAS_PREDICATE_DEF_MARKER) != nullptr;
        }

        bool isGhostMethod(llvm::Function &f)
        {
            return f.getMetadata(PALLAS_GHOST_MARKER) != nullptr;
        }

        void markGhostMethod(llvm::Function &f, llvm::LLVMContext &context)
        {
            auto idStr = llvm::MDString::get(context, "");
            f.setMetadata(PALLAS_GHOST_MARKER,
                          llvm::MDNode::get(context, idStr));
        }

        bool isSubscope(const llvm::DIScope *parentScope,
                        const llvm::DIScope *scope)
        {
            if (parentScope == nullptr || scope == nullptr)
                return false;
            if (parentScope == scope)
                return true;
            return isSubscope(parentScope, scope->getScope());
        }

        llvm::DILocalScope *getScopeOfBasicBlock(const llvm::BasicBlock &bb)
        {
            for (auto &i : bb)
            {
                // We skip phi-nodes because these are braught in from a
                // different sope when e.g. mem2reg is applied.
                if (llvm::isa<llvm::PHINode>(i))
                    continue;
                if (auto *s = getScopeOfInstruction(i))
                    return s;
            }

            return nullptr;
        }

        llvm::DILocalScope *getScopeOfInstruction(const llvm::Instruction &i)
        {
            if (auto *dbgLoc = i.getDebugLoc().get())
                if (auto *scope = dbgLoc->getScope())
                    return scope;
            return nullptr;
        }

    }

}