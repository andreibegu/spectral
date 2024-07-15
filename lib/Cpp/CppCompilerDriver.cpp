#include <fstream>
#include <sstream>

#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>

#include "CppCompilerDriver.h"
#include "PallasException.h"

namespace sourcespec2ir
{

    CppCompilerDriver::CppCompilerDriver(std::string specLibPath)
        : specLibPath(specLibPath)
    {
    }

    /*
     * TODO: Very ugly, try to use libclang or something like that.
     * TODO: Cleanup file handling (or better: avoid using files altogether)
     */
    std::unique_ptr<llvm::Module> CppCompilerDriver::compileSource(
        const std::string &srcFilePath, llvm::LLVMContext &context)
    {
        // Run compiler to generate the IR
        const std::string irFilePath = "tmp_ir_source.ll";

        std::stringstream call;
        call << " clang++-15 ";
        call << " -emit-llvm -S " ;
        call << " -g -O0 ";
        call << " -Xclang -disable-O0-optnone ";
        call << srcFilePath;
        call << " -o " + irFilePath;
        call << " -I " << specLibPath << "/";
        std::system(call.str().c_str());

        // Parse the resulting file as an LLVM-IR module
        llvm::SMDiagnostic irSMDiag;
        auto irModule = llvm::parseIRFile(irFilePath, irSMDiag, context);

        if (!irModule)
        {
            // Reading the ir-file did not work
            throw utils::PallasException(
                "Failed to compile the provided source-file.");
        }

        return irModule;
    }

}