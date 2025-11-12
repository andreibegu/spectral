#include "MetadataUtils.h"
#include "IrSpecConstants.h"

#include <llvm/IR/DebugLoc.h>

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
                if (auto *s = getScopeOfInstruction(i))
                    return s;
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