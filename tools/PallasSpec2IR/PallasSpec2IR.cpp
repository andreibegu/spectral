#include "CSpecTransformer.h"
#include "CSpecUnifier.h"
#include "CCompilerDriver.h"
#include "CppSpecTransformer.h"
#include "SwiftSpecTransformer.h"
#include "utils/OutputConfig.h"
#include "utils/PallasException.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <memory>
#include <string>

/*
 * Commandline-options
 */

// Enum for selecting the source-language from the commandline
enum ClSourceLang
{
    C,
    Cpp,
    Swift,
    Infer
};

std::map<ClSourceLang, std::string> langNames = {
    {C, "C"}, {Cpp, "C++"}, {Swift, "Swift"}, {Infer, "Auto"}};

static llvm::cl::OptionCategory SpecTransformCat(
    "Specification transformation");

static llvm::cl::opt<std::string> clInFile(
    llvm::cl::Positional, llvm::cl::desc("input-file"), llvm::cl::Required,
    llvm::cl::cat(SpecTransformCat));

static llvm::cl::opt<std::string> clOutFile(
    "o",
    llvm::cl::desc("output-file"),
    llvm::cl::value_desc("Path to out-file"),
    llvm::cl::cat(SpecTransformCat));

static llvm::cl::opt<bool> clPrintDebug(
    "d", llvm::cl::desc("Print debug info"), llvm::cl::init(false),
    llvm::cl::cat(SpecTransformCat));

static llvm::cl::opt<bool> clPrintMatching(
    "m", llvm::cl::desc("Print info about matching-stage"),
    llvm::cl::init(false), llvm::cl::cat(SpecTransformCat));

static llvm::cl::opt<bool> clPrintUnification(
    "u", llvm::cl::desc("Print info about unification-stage"),
    llvm::cl::init(false), llvm::cl::cat(SpecTransformCat));

static llvm::cl::opt<std::string> clLibPath(
    "lib", llvm::cl::desc("Path to specification-library"),
    llvm::cl::value_desc("Directory of the specification library"),
    llvm::cl::init("../../res/spec_libs/c/"),
    llvm::cl::cat(SpecTransformCat));

static llvm::cl::opt<ClSourceLang> clLang(
    "lang", llvm::cl::desc("Source-language of the input-file"),
    llvm::cl::init(Infer),
    llvm::cl::values(
        clEnumVal(C, "C"),
        clEnumVal(Cpp, "C++"),
        clEnumVal(Swift, "Swift"),
        clEnumVal(Infer, "Infer source-language from file-extension")),
    llvm::cl::cat(SpecTransformCat));

static llvm::cl::opt<std::string> clWorkDir(
    "wDir",
    llvm::cl::init(""),
    llvm::cl::desc("The working directory. If the option is not set, the current directory is used."),
    llvm::cl::value_desc("Path to the working-directory."),
    llvm::cl::cat(SpecTransformCat));


static llvm::cl::opt<bool> clMem2Reg(
    "mem2reg", llvm::cl::desc("Apply mem2reg pass to the compiled source."), llvm::cl::init(false),
    llvm::cl::cat(SpecTransformCat));

int main(int argc, char **argv)
{
    // Parse commandline-options and wrap them in struct
    llvm::cl::HideUnrelatedOptions(SpecTransformCat);
    llvm::cl::ParseCommandLineOptions(argc, argv);

    // Bundle debug-options in config-struct
    sourcespec2ir::utils::OutputConfig config(
        clPrintMatching, clPrintUnification, clPrintDebug);

    // Get the source-language. If it is not specified with the -lang option,
    // try to infer it using the file-extension of the input-file.
    ClSourceLang lang = clLang;
    if (lang == Infer)
    {
        auto inFileExt = clInFile.substr(clInFile.find_last_of("."));
        if (inFileExt == ".c")
            lang = C;
        else if (inFileExt == ".cpp")
            lang = Cpp;
        else if (inFileExt == ".swift")
            lang = Swift;
        else
        {
            throw sourcespec2ir::utils::PallasException(
                "Unable to infer the language from the file-extension. \
                 Please specify the language manually using the -lang option.");
        }
        if (config.printDebugInfo)
            llvm::outs() << "Infered source-language as: " << langNames[lang] << "\n";
    }

    if (config.printDebugInfo)
    {
        llvm::outs() << "Source-language set to " << langNames[lang] << " \n";
        llvm::outs() << "From: " << clInFile << " to: " << clOutFile << "\n";
        llvm::outs() << "Working directory: " << clWorkDir << "\n";
    }

    try
    {
        if (lang == C)
        {
            sourcespec2ir::CSpecTransformer transformer(
                std::make_unique<sourcespec2ir::CCompilerDriver>(clLibPath, clWorkDir),
                std::make_unique<sourcespec2ir::CSpecUnifier>(config.printUnificationInfo),
                clInFile, clOutFile, config, clWorkDir, clMem2Reg);
            transformer.transformModule();
        }
        else if (lang == Cpp)
        {
            sourcespec2ir::CppSpecTransformer transformer(
                clLibPath, clInFile, clOutFile, config, clWorkDir, clMem2Reg);
            transformer.transformModule();
        }
        else if (lang == Swift)
        {
            sourcespec2ir::SwiftSpecTransformer transformer(
                clLibPath, clInFile, clOutFile, config, clWorkDir, clMem2Reg);
            transformer.transformModule();
        } 
    }
    catch (const sourcespec2ir::utils::PallasException &e)
    {
        // We catch all exception that are based on Pallas exeption.
        // We print the error message and return a non-zero return-value to
        // allow for lit-tests that expect the program to fail.
        llvm::errs() << "===============================================\n";
        llvm::errs() << "ERROR: An error was encountered during the"
                     << " specification transformation: \n";
        llvm::errs() << e.what() << "\n";
        llvm::errs() << "===============================================\n";
        return 1;
    }

    return 0;
}