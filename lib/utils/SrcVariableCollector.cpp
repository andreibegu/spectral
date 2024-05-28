
#include <iostream>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>

#include "utils/SrcVariableCollector.h"

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

    void printAllSrcVars(std::string srcFunctionName, std::string irFilePath)
    {
        // TODO: Modify to accept a module instead of a file-name.

        // Parse the given IR-file
        llvm::LLVMContext context;
        llvm::SMDiagnostic smDiag;
        auto pModule = llvm::parseIRFile(irFilePath, smDiag, context);

        for (const auto &f : pModule->getFunctionList())
        {
            if (f.isIntrinsic())
            {
                // Skip llvm-intrinsics
                continue;
            }

            // Look at the debug-metadata to find the source-name of the function.
            auto pDISubprog = f.getSubprogram();
            if (pDISubprog != nullptr)
            {
                auto fName = pDISubprog->getName().str();
                std::cout << "Detected function with name: " << fName << std::endl;
                auto vars = getSrcVariablesForFunction(f, true);
                for (auto &v: *vars) {
                    std::cout << "Variable " << v.first().str() << " : " << v.second << std::endl;
                } 
            }
            else
            {
                // Should no happen for now. In the future we need to consider the specification-functions.
                std::cout << "No DISubprogram found for function. Skipping. " << std::endl;
            }
        }
    }

    std::unique_ptr<llvm::StringMap<std::string>> getSrcVariablesForFunction(const llvm::Function &fun, const bool argsOnly)
    {
        if (fun.isDeclaration())
        {
            // This is an external declaration, not a definition
            throw std::invalid_argument("Recieved a function declaration instead of a function definition");
        }

        // Map of source-variable names to their types.
        auto sourceVarMap = std::make_unique<llvm::StringMap<std::string>>();

        // Iterate over all of the llvm.dbg.-intrinsics and get the names and types (of the function arguments)
        for (auto i = llvm::inst_begin(fun), end = llvm::inst_end(fun); i != end; ++i)
        {
            const llvm::Instruction *instr = &*i;
            // Only consider calls to llvm intrinsics
            if (auto *intrinsic = llvm::dyn_cast<llvm::DbgVariableIntrinsic>(instr))
            {
                const auto *srcVar = intrinsic->getVariable();
                auto srcName = srcVar->getName().str();
                auto srcType = srcVar->getType()->getName().str();

                // Filter on function arguments (Assumes that intrinsic is directly attached to argument --> might require mem2reg)
                if (argsOnly && !isFunctionArgument(intrinsic))
                {
                    continue;
                }

                // Update result-map and check for consistency
                auto currentVal = sourceVarMap->find(srcName);
                if (currentVal != sourceVarMap->end() && currentVal->second != srcType)
                {
                    throw std::runtime_error("Found two different types for the same variable. This should not happen!");
                }
                sourceVarMap->insert(std::make_pair(srcName, srcType));
            }
        }

        return sourceVarMap;
    }

    llvm::Function *getFunctionBySrcName(llvm::Module &module, const std::string &src_name)
    {
        for (auto &f : module.getFunctionList())
        {
            // Skip llvm-intrinsics
            if (f.isIntrinsic())
            {
                continue;
            }

            // Look at the debug-metadata to find the source-name of the function.
            auto pDISubprog = f.getSubprogram();
            if (pDISubprog != nullptr)
            {
                auto fName = pDISubprog->getName().str();
                if (fName == src_name) 
                {
                    return &f;
                }
            }
        }

        return nullptr;
    }

}