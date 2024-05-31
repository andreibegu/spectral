#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <stdio.h>

#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>

#include "transform/CCompilerDriver.h"

namespace sourcespec2ir
{

    /*
     * TODO: Very ugly, try to use libclang or something like that.
     * TODO: Cleanup file handling (or better: avoid using files altogether)
     */

    std::unique_ptr<llvm::Module> CCompilerDriver::compileSource(const std::string &srcFilePath, llvm::LLVMContext &context)
    {
        // Run compiler to generate the IR
        // srcFilePath.substr(0, srcFilePath.find_last_of(".")) + "_tmp.ll"; // 
        const std::string irFilePath = "tmp_ir_source.ll";

        // TODO: Handle include-headers, ...
        std::string clangCall = "clang-15 -emit-llvm -S -g -O0 " + srcFilePath +
                                " -o " + irFilePath +
                                " -Xclang -disable-O0-optnone";
        std::system(clangCall.c_str());

        // Parse the resulting file as an LLVM-IR module
        llvm::SMDiagnostic irSMDiag;
        auto irModule = llvm::parseIRFile(irFilePath, irSMDiag, context);

        if (!irModule)
        {
            // Reading the ir-file did not work
            throw std::runtime_error("Failed to compile the provided source-file.");
        }

        return irModule;
    }

    std::unique_ptr<llvm::Module> CCompilerDriver::compileSpecification(const std::string &cu, llvm::LLVMContext &context)
    {
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
        llvm::SMDiagnostic irSMDiag;
        auto irModule = llvm::parseIRFile(tmpIrFile, irSMDiag, context);

        if (!irModule)
        {
            // Reading the ir-file did not work
            throw std::runtime_error("Failed to read the ir of the compiled specification-expression.");
        }

        return irModule;
    }

}