#include "transform/SourceSpecTransformer.h"
#include "transform/CSpecUnifier.h"
#include "transform/CCompilerDriver.h"
#include "utils/PallasException.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <iostream>

/*
 * Commandline-options
 */
static llvm::cl::opt<std::string> clInFile(llvm::cl::Positional, llvm::cl::desc("input-file"), llvm::cl::Required);

static llvm::cl::opt<std::string> clOutFile("o", llvm::cl::desc("output-file"), llvm::cl::value_desc("filename"));

static llvm::cl::opt<bool> clPrintDebug("d", llvm::cl::desc("Print debug info"), llvm::cl::init(false));

int main(int argc, char **argv)
{
    llvm::cl::ParseCommandLineOptions(argc, argv);

    std::cout << "From: " << clInFile << " to: " << clOutFile << std::endl;

    try
    {
        auto compilerDriver = std::make_unique<sourcespec2ir::CCompilerDriver>();
        auto specUnifier = std::make_unique<sourcespec2ir::CSpecUnifier>();
        sourcespec2ir::SourceSpecTransformer transformer(
            std::move(compilerDriver),
            std::move(specUnifier),
            clInFile.c_str(),
            clOutFile.c_str(),
            clPrintDebug);
        transformer.transformModule();
    } catch (const sourcespec2ir::utils::PallasException &e)
    {
        // We catch all exception that are based on Pallas exeption. 
        // We print the error message and return a non-zero return-value to 
        // allow for lit-tests that expect the program to fail. 
        llvm::errs() << "===============================================\n";
        llvm::errs() << "ERROR: An error was encountered during the" <<
            " specification transformation: \n";
        llvm::errs() << e.what() << "\n";
        llvm::errs() << "===============================================\n";
        return 1;
    }

    return 0;
}