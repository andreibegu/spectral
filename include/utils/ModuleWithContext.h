#ifndef PALLAS_MODULEWITHCONTEXT_H
#define PALLAS_MODULEWITHCONTEXT_H

#include <memory>

#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>

namespace sourcespec2ir
{
    /**
     * A simple struct that groups unique-pointers to a llvm-module with the pointert to the correponding 
     * LLVMContext and SMDiagnostic objects. 
     */

    struct ModuleWithContext
    {
        std::unique_ptr<llvm::LLVMContext> llvmContext;
        std::unique_ptr<llvm::SMDiagnostic> llvmSMDiagnostic;
        std::unique_ptr<llvm::Module> llvmModule;
    };
    

}

#endif // PALLAS_MODULEWITHCONTEXT_H