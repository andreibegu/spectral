#include <fstream>
#include <sstream>

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>

#include "CppCompilerDriver.h"
#include "PallasException.h"

namespace sourcespec2ir
{

    CppCompilerDriver::CppCompilerDriver(std::string specLibPath,
                                         const std::string &workingDir)
        : SourceCompilerDriver(workingDir),
          specLibPath(specLibPath)
    {
    }

    std::unique_ptr<llvm::Module> CppCompilerDriver::compileSource(
        const std::string &srcFilePath, llvm::LLVMContext &context)
    {
        // Generate a file for the compiled ir in the working directory
        // const std::string irFileName = "tmp_ir_source.ll";
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
        std::stringstream call;
        call << " clang++-17 ";
        call << " -emit-llvm -S ";
        call << " -g -O0 ";
        call << " -Xclang -disable-O0-optnone ";
        call << " -Werror ";
        call << srcFilePath;
        call << " -o " + irOutStr;
        call << " -I " << specLibPath << "/";
        auto eCode = std::system(call.str().c_str());
        if (eCode != 0)
            throw utils::PallasException(
                "An error occured while compiling the source-file.");

        // Parse the resulting file as an LLVM-IR module
        llvm::SMDiagnostic irSMDiag;
        auto irModule = llvm::parseIRFile(irOutStr, irSMDiag, context);

        if (!irModule)
        {
            // Reading the ir-file did not work
            throw utils::PallasException(
                "Failed to compile the provided source-file.");
        }

        return irModule;
    }

}