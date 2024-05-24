#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <stdio.h> 

#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>

#include "transform/CCompilerDriver.h"

namespace sourcespec2ir {

    std::unique_ptr<llvm::Module> CCompilerDriver::compileSpecification(const std::string &cu, llvm::LLVMContext &context)
    {
        // TODO: Very ugly, try to use libclang or something like that.
        // TODO: Cleanup file handling (or better: avoid using files altogether)

        // Put c-code into file
        std::ofstream out("tmp_driver.c");
        out << cu;
        out.close();
        
        // Run compiler to generate IR
        // std::cout << "Invoking clang " << std::endl;
        std::system("clang-15 -emit-llvm -S -g -O0 tmp_driver.c -o tmp_driver.ll -Xclang -disable-O0-optnone");
        
        // TODO: Run the opt-tool for the mem2reg-pass

        // Parse the resulting file as an LLVM-IR module
        // TODO: Check if the SMDiagnostics has to stay in scope along with the module.
        llvm::SMDiagnostic irSMDiag;
        // auto irSMDiag = std::make_unique<llvm::SMDiagnostic>();
        auto irModule = llvm::parseIRFile("tmp_driver.ll", irSMDiag, context);

        if (!irModule) 
        {   
            // Reading the ir-file did not work
            throw std::runtime_error("Failed to read the ir of the compiled specification-expression.");
        }

        return irModule;
    }

}