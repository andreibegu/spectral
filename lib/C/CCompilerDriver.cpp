#include <stdlib.h>
#include <fstream>
#include <stdio.h>

#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>

#include "CCompilerDriver.h"
#include "PallasException.h"

namespace sourcespec2ir
{

    CCompilerDriver::CCompilerDriver(std::string specLibPath)
        : specLibPath(specLibPath)
    {
    }

    /*
     * TODO: Very ugly, try to use libclang or something like that.
     * TODO: Cleanup file handling (or better: avoid using files altogether)
     */

    std::unique_ptr<llvm::Module> CCompilerDriver::compileSource(
        const std::string &srcFilePath, llvm::LLVMContext &context)
    {
        // Run compiler to generate the IR
        // srcFilePath.substr(0, srcFilePath.find_last_of(".")) + "_tmp.ll"; //
        const std::string irFilePath = "tmp_ir_source.ll";

        std::string clangCall = "clang-15 -emit-llvm -S -g -O0 " + srcFilePath +
                                " -o " + irFilePath +
                                " -Xclang -disable-O0-optnone" + 
                                " -I " + specLibPath +"/" ;
        std::system(clangCall.c_str());

        // Parse the resulting file as an LLVM-IR module
        llvm::SMDiagnostic irSMDiag;
        auto irModule = llvm::parseIRFile(irFilePath, irSMDiag, context);

        if (!irModule)
        {
            // Reading the ir-file did not work
            throw utils::PallasException("Failed to compile the provided source-file.");
        }

        return irModule;
    }
}