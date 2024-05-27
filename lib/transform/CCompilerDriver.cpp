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
        const std::string tmpFileBase = "tmp_driver";
        const std::string tmpSrcFile = tmpFileBase + ".c";
        const std::string tmpIrFile = tmpFileBase + ".ll";

        // Put c-code into file
        std::ofstream out(tmpSrcFile);
        out << cu;
        out.close();
        
        // Run compiler to generate IR
        // std::cout << "Invoking clang " << std::endl;
        std::string clangCall = "clang-15 -emit-llvm -S -g -O0 " + tmpSrcFile + 
            " -o " + tmpIrFile + 
            " -Xclang -disable-O0-optnone" + 
            " -I ../../res/spec_libs/c/";
        std::system(clangCall.c_str());

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