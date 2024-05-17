
#include <iostream>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>

#include "utils/IRVariableCollector.h"

namespace sourcespec2ir
{

    IRVariableCollector::IRVariableCollector(std::string srcFunctionName, std::string irFilePath)
    {
        // TODO: Modify to accept a module instead of a file-name.
        // TODO: Report a name for the return variable.

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

    std::unique_ptr<llvm::StringMap<std::string>> IRVariableCollector::getVariablesForFunction(const llvm::Function &fun)
    {
        if (fun.isDeclaration())
        {
            // This is an external declaration, not a definition
            std::cout << "Declaration: " << fun.getName().str() << std::endl;
            throw std::invalid_argument("Recieved a function declaration instead of a function definition");
        }

        // Map of source-variable names to their types.
        auto sourceVarMap = std::make_unique<llvm::StringMap<std::string>>();

        // Collect the IR-level arguments into a set
        llvm::SmallPtrSet<const llvm::Argument*, 10> argSet;
        for (auto &arg : fun.args())
        {
            argSet.insert(&arg);
        }

        // Iterate over all of the llvm.dbg.-intrinsics and get the names and types of the function arguments
        for (auto i = llvm::inst_begin(fun), end = llvm::inst_end(fun); i != end; ++i)
        {
            const llvm::Instruction* instr = &*i;
            // Only consider calls to llvm intrinsics (llvm.dbg.declare)
            if (llvm::isa_and_nonnull<llvm::IntrinsicInst>(instr))
            {
                const llvm::IntrinsicInst* intrinsic = llvm::cast<llvm::IntrinsicInst>(instr);
                if (intrinsic->getCalledFunction() != nullptr && 
                    intrinsic->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::dbg_declare)
                {
                    // TODO: Filter on function arguments
                    // Potential alternative: Filter on scopes ???

                    // Handle calls to llvm.dbg.declare
                    // The second argument of the function contains the metadata-description
                    const auto *op2AsMd = llvm::cast<llvm::MetadataAsValue>(intrinsic->getOperand(1))->getMetadata();
                    const auto *varMd = llvm::cast<llvm::DIVariable>(op2AsMd); 
                    auto srcName = varMd->getName().str();
                    auto srcType = varMd->getType()->getName().str();
                    // TODO: Differentiate local, and global variables ?!
                    std::cout << "Detected variable: " << srcName << " : " << srcType << std::endl;
                }
                // TODO handle dbg_value. dbg_assign
            }
        }

        // TODO: Wrap the return-type of the function in an appropriate variable?!
        return sourceVarMap;
    }

}