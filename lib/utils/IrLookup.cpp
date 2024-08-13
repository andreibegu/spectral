#include "IrLookup.h"
#include "PallasException.h"

#include <stdexcept>

#include <llvm/IR/Attributes.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IntrinsicInst.h>

namespace sourcespec2ir
{
    namespace utils
    {
        std::unique_ptr<std::set<llvm::DILocalVariable *>>
        getSrcVariablesForFunction(const llvm::Function &fun)
        {
            if (fun.isDeclaration())
            {
                // This is an external declaration, not a definition
                throw utils::PallasException(
                    "Function declarations are not supported.");
            }

            auto srcVarSet = std::make_unique<std::set<llvm::DILocalVariable *>>();

            // Iterate over all of the llvm.dbg.-intrinsics and get the names and types of the variables
            for (auto i = llvm::inst_begin(fun), end = llvm::inst_end(fun); i != end; ++i)
            {
                const llvm::Instruction *instr = &*i;
                // Only consider calls to llvm intrinsics
                if (auto *intrinsic = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(instr))
                {
                    auto *srcVar = intrinsic->getVariable();
                    // Filter on function arguments
                    if (srcVar->getArg() != 0)
                    {
                        srcVarSet->insert(srcVar);
                    }
                }
            }

            return srcVarSet;
        }

        std::unique_ptr<std::set<llvm::DILocalVariable *>> getVarsForLoopInv(
            llvm::Function *parentFunc, const llvm::BasicBlock *loopHeader)
        {
            auto varSet = std::make_unique<std::set<llvm::DILocalVariable *>>();
            const llvm::DominatorTree domTree(*parentFunc);

            // Get all llvm.dbg.-intrinsics that dominate the loop
            for (auto i = llvm::inst_begin(parentFunc),
                      end = llvm::inst_end(parentFunc);
                 i != end; ++i)
            {
                if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*i))
                {
                    if (!domTree.dominates(intr, loopHeader))
                        continue;
                    auto *srcVar = intr->getVariable();
                    varSet->insert(srcVar);
                }
            }

            return varSet;
        }

        std::unique_ptr<std::set<llvm::DILocalVariable *>> getVarsForStatement(
            llvm::Instruction *inst)
        {
            // Get the parent-function of the instruction
            if (inst == nullptr ||
                inst->getParent() == nullptr ||
                inst->getParent()->getParent() == nullptr)
            {
                throw utils::PallasException(
                    "Encountered instruction without parent");
            }
            auto parentFunc = inst->getParent()->getParent();

            // Collect all variables from the llvm.dbg.-intrinsics that 
            // dominate the instruction
            auto varSet = std::make_unique<std::set<llvm::DILocalVariable *>>();
            const llvm::DominatorTree domTree(*parentFunc);
            for (auto i = llvm::inst_begin(parentFunc),
                      end = llvm::inst_end(parentFunc);
                 i != end; ++i)
            {
                if (&*i == inst) 
                    continue;

                if (auto *intr = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&*i))
                {
                    if (!domTree.dominates(intr, inst))
                        continue;
                    auto *srcVar = intr->getVariable();
                    varSet->insert(srcVar);
                }
            }

            return varSet;
        }

        llvm::Type *getReturnType(const llvm::Function &f)
        {
            if (f.hasParamAttribute(0, llvm::Attribute::StructRet))
                return f.getParamStructRetType(0);
            if (f.hasParamAttribute(1, llvm::Attribute::StructRet))
                return f.getParamStructRetType(1);
            return f.getReturnType();
        }

    }

}