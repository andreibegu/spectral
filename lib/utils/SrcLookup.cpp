#include "SrcLookup.h"

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

    }
}