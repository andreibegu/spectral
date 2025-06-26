#include "PallasException.h"
#include "SrcLookup.h"

#include <llvm/IR/Argument.h>
#include <llvm/IR/IntrinsicInst.h>

namespace sourcespec2ir
{
    namespace utils
    {

        llvm::Function *getFunctionBySrcName(
            llvm::Module &module, const std::string &src_name)
        {
            for (auto &f : module.getFunctionList())
            {
                // Skip llvm-intrinsics
                if (f.isIntrinsic())
                    continue;
                // Compare source-name of function
                auto fName = getSrcNameOfFunction(f);
                if (!fName.empty() && fName == src_name)
                    return &f;
            }
            return nullptr;
        }

        std::string getSrcNameOfFunction(const llvm::Function &f)
        {
            auto pDISubprog = f.getSubprogram();
            if (pDISubprog != nullptr)
                return pDISubprog->getName().str();
            else
                return f.getName().str();
        }

        std::string getSrcVarName(const llvm::DILocalVariable &var)
        {
            return var.getName().str();
        }

    }
}