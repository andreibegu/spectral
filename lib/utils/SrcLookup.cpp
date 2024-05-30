/*

#include <llvm/IR/Metadata.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Module.h>


#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
*/

#include "utils/SrcLookup.h"
#include "utils/IrLookup.h"

#include <iostream>

#include <llvm/IR/Argument.h>
#include <llvm/IR/IntrinsicInst.h>

namespace sourcespec2ir
{
    namespace utils
    {

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

        std::string getSrcVarName(const llvm::DILocalVariable &var) 
        {
            return var.getName().str();
        }

        std::string getSrcVarType(const llvm::DILocalVariable &var)
        {
            return var.getType()->getName().str();
        }

        void printAllSrcVars(const llvm::Module &module)
        {

            for (const auto &f : module.getFunctionList())
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
                    auto vars = utils::getSrcVariablesForFunction(f, true);
                    for (auto v : *vars)
                    {
                        auto srcName = utils::getSrcVarName(*v);
                        auto srcType = utils::getSrcVarType(*v);
                        std::cout << "Variable " << srcName << " : " << srcType << std::endl;
                    }
                }
                else
                {
                    // Should no happen for now. In the future we need to consider the specification-functions.
                    std::cout << "No DISubprogram found for function. Skipping. " << std::endl;
                }
            }
        }

    }
}