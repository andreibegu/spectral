
#include <iostream>
#include <llvm/IR/Argument.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/Intrinsics.h>

#include "utils/IRVariableCollector.h"

namespace sourcespec2ir
{

    IRVariableCollector::IRVariableCollector(std::string srcFunctionName, std::string irFilePath)
    {

        // Parse the given IR-file
        llvm::LLVMContext context;
        llvm::SMDiagnostic smDiag;
        auto pModule = llvm::parseIRFile(irFilePath, smDiag, context);

        // TODO: Extract looking up a function by it's source-name into a separate class/ function.

        for (const auto &f : pModule->getFunctionList())
        {
            if (f.isIntrinsic())
            {
                // Skip llvm-intrinsics
                std::cout << "Skipping intrinsic " << llvm::Intrinsic::getName(f.getIntrinsicID()).str() << std::endl;
                continue;
            }

            // Look at the debug-metadata to find the source-name of the function.
            auto pDISubprog = f.getSubprogram();
            if (pDISubprog != nullptr)
            {
                auto fName = pDISubprog->getName().str();
                std::cout << "Detected function with name: " << fName << std::endl;
                getVariablesForFunction(f);
            }
            else
            {
                // Should no happen for now. In the future we need to consider the specification-functions.
                std::cout << "No DISubprogram found for function. Skipping. " << std::endl;
            }
        }
    }

    void IRVariableCollector::getVariablesForFunction(const llvm::Function &fun)
    {
        // TODO: Iterate over all of the llvm.dbg.declare-intrinsics and get the names and types.

        // TODO: Wrap the return-type of the function in an appropriate variable?!

        // Get the function arguments
        for (auto arg = fun.arg_begin(); arg != fun.arg_end(); arg++) {
            std::cout << "Param: " << arg->getName().str() << std::endl;
        }
        // TODO: Do smart things
    }

}