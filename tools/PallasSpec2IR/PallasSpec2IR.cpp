#include "transform/CSpecTransformer.h"
#include "transform/CSpecUnifier.h"
#include "transform/CCompilerDriver.h"
#include "utils/OutputConfig.h"
#include "utils/PallasException.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>

/*
 * Commandline-options
 */
static llvm::cl::opt<std::string> clInFile(
    llvm::cl::Positional, llvm::cl::desc("input-file"), llvm::cl::Required);

static llvm::cl::opt<std::string> clOutFile(
    "o", llvm::cl::desc("output-file"), llvm::cl::value_desc("filename"));

static llvm::cl::opt<bool> clPrintDebug(
    "d", llvm::cl::desc("Print debug info"), llvm::cl::init(false));

static llvm::cl::opt<bool> clPrintMatching(
    "m", llvm::cl::desc("Print info specification matching info"),
    llvm::cl::init(false));

static llvm::cl::opt<std::string> clLibPath(
    "lib", llvm::cl::desc("Path to specification-library"),
    llvm::cl::value_desc("Directory of the specification library"),
    llvm::cl::init("../../res/spec_libs/c/"));

int main(int argc, char **argv)
{
    // Parse commandline-options and wrap them in struct
    llvm::cl::ParseCommandLineOptions(argc, argv);
    sourcespec2ir::utils::OutputConfig config(
        clPrintMatching, clPrintDebug);

    if (config.printDebugInfo)
    {
        llvm::outs() << "From: " << clInFile << " to: " << clOutFile << "\n";
    }

    try
    {
        auto compilerDriver = std::make_unique<sourcespec2ir::CCompilerDriver>(
            clLibPath);
        auto specUnifier = std::make_unique<sourcespec2ir::CSpecUnifier>();
        sourcespec2ir::CSpecTransformer transformer(
            std::move(compilerDriver),
            std::move(specUnifier), clInFile, clOutFile,
            config);
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