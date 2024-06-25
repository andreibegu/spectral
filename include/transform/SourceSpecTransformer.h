#ifndef PALLAS_SOURCESPECTRANSFORMER_H
#define PALLAS_SOURCESPECTRANSFORMER_H

#include <string>
#include <fstream>
#include <memory>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"

#include "SourceSpecMatcher.h"
#include "SourceCompilerDriver.h"
#include "SpecUnifier.h"
#include "wrapper/IRWrapperFunction.h"
#include "wrapper/SrcWrapperFunction.h"

namespace sourcespec2ir
{

    class SourceSpecTransformer
    {
    public:
        SourceSpecTransformer(std::unique_ptr<SourceCompilerDriver> compilerDriver,
                              std::unique_ptr<SpecUnifier> specUnifier,
                              const char *srcInputPath,
                              const char *outputPath,
                              const bool printDebugInfo);

        void transformModule();

    protected:
        /*
         *  Factory methods
         */

        // TODO: Make virtual, add path to source-file as argument if required.
        std::unique_ptr<SourceSpecMatcher> createSpecMatcher(SpecParser::FileContext *specParseTree,
                                                             llvm::Module *irModule);

    private:
        const char *outputPath;
        const bool printDebugInfo;

        /* Counter for generating names for the specification functions.
         * TODO: Replace with something more elaborate that takes exisiting names into account.
         */
        int specFunCnt = 0;

        std::unique_ptr<SourceCompilerDriver> srcCompilerDriver;

        std::unique_ptr<SpecUnifier> specUnifier;

        // Pointer to the LLVM context
        std::unique_ptr<llvm::LLVMContext> irContext;

        // Pointer to the loaded llvm-ir module
        std::unique_ptr<llvm::Module> irModule;

        /**
         * Pointers to stuff that is required to keep the parse-tree of the source-file in scope.
         */
        std::unique_ptr<std::ifstream> srcFileStream;
        std::unique_ptr<antlr4::ANTLRInputStream> srcInStream;
        std::unique_ptr<SpecLexer> srcLexer;
        std::unique_ptr<antlr4::CommonTokenStream> srcTokens;
        std::unique_ptr<SpecParser> srcParser;
        SpecParser::FileContext *srcParseTree;

        /**
         * Analysis managers and pass builder that are used to run optimization-passes on the loaded IR
         */
        llvm::LoopAnalysisManager lam;
        llvm::FunctionAnalysisManager fam;
        llvm::CGSCCAnalysisManager cgam;
        llvm::ModuleAnalysisManager mam;

        // While we do not use the PassBuilder directly, it is required by the ModuleAnalysisManager
        // Causes SegmentationFault otherwise.
        llvm::PassBuilder passBuilder;

        /**
         * Contents of the declaration-block of the specifications.
         */
        std::unique_ptr<llvm::SmallVector<std::string>> specDeclarations;

        /**
         * Transforms an expression in the source language into LLVM-IR by wrapping
         * it into a function. The generated function is added to the module of the
         * SourceSpecTransformer.
         * Returns an IRWrapper function that bundles a pointer to the newly
         * generated function along with a list of the metadata-nodes that
         * describe the functions parameter-list.
         */
        std::unique_ptr<IRWrapperFunction> transformExpression(
            std::string expr, const std::set<llvm::DILocalVariable *> &srcVars);

        /**
         * Runs the mem2reg-pass on the given module.
         */
        void promoteMemToReg(llvm::Module &module);

        /**
         * Encodes a block of function-contract statements and adds the 
         * generated MD-nodes to the module.
         */
        void encodeFunctionContractBlock(const MatchedFunctionContract &contract);

        /**
         * Encodes a statement of a function contract into a metadata-node.
         * Returns a pointer to the new MD-node. Ownership of the new node
         * lies with the llvm-context.
         */
        llvm::MDNode *encodeFunctionContractStatement(
            const llvm::Function &function,
            const MatchedFContractStatement &stmnt);

        /**
         * Encodes a block of loop-invariants and adds the generated MD-nodes
         * to the module.
         */
        void encodeLoopInvBlock(const MatchedLoopInvariantBlock &invBlock);

        /**
         * Encodes a single loop-invariant into a metadata-node.
         * The new Metadata node is added to the back of the given vector.
         */
        void encodeLoopInv(std::string specExpr,
                           const std::set<llvm::DILocalVariable *> &vars,
                           llvm::SmallVector<llvm::Metadata *> &nodes);

        /**
         * Write the IR-representation of the given module into
         * a file at the given path.
         */
        void writeToFile(const llvm::Module &module, const char *outPath);

        /**
         * Takes a string that contains a source-language expression along with a
         * sequence of DILocalVariable that describe the source-variables that
         * are accessible in the expression.
         * The generated wrapping-function MUST use the same ordering of it's
         * arguments as is defined by parameter srcVars.
         *
         * TODO: Move somewhere else or make virtual as this is source-language specific
         */
        std::unique_ptr<SrcWrapperFunction> getWrappingFunction(
            llvm::ArrayRef<llvm::DILocalVariable *> srcVars,
            const std::string &expr);

        /**
         * Takes a string-representation of a source-labguage function and turns 
         * it into a full compilation unit in the source language.
         * E.g. adds imports for the specification library, ...
         *
         * TODO: Move somewhere else or make virtual as this is source-language specific
         */
        std::string assembleSrcCompilationUnit(const SrcWrapperFunction &wrapperFunction);

        /**
         * Returns a string representation of the given DIType that matches the 
         * types representation on the source-level.
         * If the stirng-representation cannot be generated, an error is raised.
         *
         * TODO: Move somewhere else or make virtual as this is source-language specific
         *      Or even better: Extract the source-language specific aspects.
         */
        std::string getSrcReprOfType(const llvm::DIType *type);
    };

}

#endif // PALLAS_SOURCESPECTRANSFORMER_H