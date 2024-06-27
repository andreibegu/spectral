#include "utils/IrLookup.h"
#include "utils/PallasException.h"

#include <stdexcept>

#include <llvm/IR/Dominators.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IntrinsicInst.h>

namespace
{

    /**
     * Checks if the debug intrinsic refers to a function argument.
     *
     * TODO: Include global variables
     * TODO: Support llvm.dbg.declare
     */
    bool isFunctionArgument(const llvm::DbgVariableIntrinsic *intrinsic)
    {
        if (auto *dbgIntr = llvm::dyn_cast<llvm::DbgValueInst>(intrinsic))
        {
            if (llvm::isa<llvm::Argument>(dbgIntr->getValue()))
            {
                return true;
            }
        }
        // Currently only arguments that are referenced using llvm.dbg.value are supported.
        // This should be the case after running the mem2reg pass.
        // In the future it would be nice to support the broader case where we handle the llvm.dbg.declare that
        // contains an (indirect) pointer to the argument.
        return false;
    }

}

namespace sourcespec2ir
{
    namespace utils
    {
        std::unique_ptr<std::set<llvm::DILocalVariable *>> getSrcVariablesForFunction(const llvm::Function &fun, const bool argsOnly)
        {
            if (fun.isDeclaration())
            {
                // This is an external declaration, not a definition
                throw utils::PallasException("Recieved a function declaration instead of a function definition");
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

                    // Filter on function arguments (Assumes that intrinsic is directly attached to argument --> might require mem2reg)
                    if (argsOnly && !isFunctionArgument(intrinsic))
                    {
                        continue;
                    }

                    // Update result-set and check for consistency
                    srcVarSet->insert(srcVar);
                }
            }

            return srcVarSet;
        }

        std::unique_ptr<std::set<llvm::DILocalVariable*>> getVarsForLoopInv(
           llvm::Function *parentFunc, const llvm::BasicBlock *loopHeader)
        {
            auto varSet = std::make_unique<std::set<llvm::DILocalVariable *>>();
            const llvm::DominatorTree domTree(*parentFunc);

            // Get all llvm.dbg.-intrinsics that dominate the loop
            for (auto i = llvm::inst_begin(parentFunc), 
                      end = llvm::inst_end(parentFunc); i != end; ++i)
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

    }

}