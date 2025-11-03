#include <stdlib.h>
#include <fstream>
#include <stdio.h>

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>

#include "CCompilerDriver.h"
#include "PallasException.h"

namespace sourcespec2ir
{

    CCompilerDriver::CCompilerDriver(std::string specLibPath,
                                     const std::string &workingDir)
        : SourceCompilerDriver(workingDir),
          specLibPath(specLibPath)
    {
    }

    std::unique_ptr<llvm::Module> CCompilerDriver::compileSource(
        const std::string &srcFilePath, llvm::LLVMContext &context)
    {
        // Generate a file for the compiled ir in the working directory
        std::string irFileName = "tmp_ir_source" +
                                 std::to_string(callCounter++) + ".ll";
        auto irOutPath = llvm::SmallString<128>(workingDir);
        llvm::sys::path::append(irOutPath, irFileName);
        auto irOutStr = irOutPath.str().str();

        // If the target-file already exists, delete it
        auto rmSucc = llvm::sys::fs::remove(irOutStr);
        if (rmSucc)
            throw utils::PallasException("Failed to remove existing ir-file.");

        // Run compiler to generate the IR
        std::string clangCall = "clang-17 -emit-llvm -S -g -O0 " + srcFilePath +
                                " -o " + irOutStr +
                                " -Xclang -disable-O0-optnone" +
                                " -I " + specLibPath + "/" + 
                                " -Xclang -disable-llvm-passes " +
                                " -fdebug-compilation-dir=. " + // Use relative paths in the debug-info
                                " -Werror " ;
        auto eCode = std::system(clangCall.c_str());
        if (eCode != 0)
            throw utils::PallasException(
                "An error occured while compiling the source-file.");

        // Parse the resulting file as an LLVM-IR module
        llvm::SMDiagnostic irSMDiag;
        auto irModule = llvm::parseIRFile(irOutStr, irSMDiag, context);

        if (!irModule)
        {
            // Reading the ir-file did not work
            throw utils::PallasException("Failed to compile the provided source-file.");
        }

        return irModule;
    }
}