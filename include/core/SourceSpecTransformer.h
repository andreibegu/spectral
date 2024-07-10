#ifndef PALLAS_SOURCESPECTRANSFORMER_H
#define PALLAS_SOURCESPECTRANSFORMER_H

#include <string>
#include <fstream>
#include <map>
#include <memory>

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
#include "SourceWrapperGenerator.h"
#include "SpecUnifier.h"
#include "OutputConfig.h"
#include "wrapper/IRWrapperFunction.h"
#include "wrapper/SrcWrapperFunction.h"

namespace sourcespec2ir
{

    class SourceSpecTransformer
    {
    public:
        SourceSpecTransformer(
            std::unique_ptr<SourceCompilerDriver> compilerDriver,
            std::unique_ptr<SpecUnifier> specUnifier,
            std::string srcInPath, std::string outPath,
            const utils::OutputConfig &outputConfig);

        void transformModule();

    protected:
        /*
         * Factory methods.
         * These can/ have to be overwritten by subclasses to create a
         * SourceSpecTransformer that can handle a specific language.
         */
        virtual std::unique_ptr<SourceWrapperGenerator> createWrapperGenerator() = 0;

        /**
         * Creates a new SpecMatcher. Uses the IR-based spec-matcher by default.
         * Can be overwritten by subclasses to use a different matching strategy.
         */
        std::unique_ptr<SourceSpecMatcher> createSpecMatcher(
            SpecParser::FileContext *specParseTree, llvm::Module *irModule);

    private:
        std::string srcInPath;
        std::string outPath;
        const utils::OutputConfig &outputConfig;

        std::unique_ptr<SourceWrapperGenerator> wrapperGenerator;

        std::unique_ptr<SourceCompilerDriver> srcCompilerDriver;

        std::unique_ptr<SpecUnifier> specUnifier;

        // Pointer to the LLVM context
        std::unique_ptr<llvm::LLVMContext> irContext;

        // Pointer to the loaded llvm-ir module
        std::unique_ptr<llvm::Module> irModule;

        /**
         * Pointers to stuff that is required to keep the parse-tree of the
         * source-file in scope.
         */
        std::unique_ptr<std::ifstream> srcFileStream;
        std::unique_ptr<antlr4::ANTLRInputStream> srcInStream;
        std::unique_ptr<SpecLexer> srcLexer;
        std::unique_ptr<antlr4::CommonTokenStream> srcTokens;
        std::unique_ptr<SpecParser> srcParser;
        SpecParser::FileContext *srcParseTree;

        /**
         * Analysis managers and pass builder that are used to run
         * optimization- and analysis passes on the loaded IR
         */
        llvm::LoopAnalysisManager lam;
        llvm::FunctionAnalysisManager fam;
        llvm::CGSCCAnalysisManager cgam;
        llvm::ModuleAnalysisManager mam;

        // While we do not use the PassBuilder directly, it is required by the
        // ModuleAnalysisManager. Causes SegmentationFault otherwise.
        llvm::PassBuilder passBuilder;

        /**
         * Contents of the declaration-block of the specifications.
         * 
         * TODO: This probably does not need to be a member-variable in the 
         *       long term.
         */
        std::unique_ptr<llvm::SmallVector<SpecDeclaration>> specDeclarations;

        /**
         * Matching information about the specifications
         */
        std::unique_ptr<sourcespec2ir::MatchedSpecifications> matchedSpecs = nullptr;

        /**
         * Maps that relate the matched specifications to the corresponding
         * wrapper-functions.
         * TODO: Check if these need to be members, or if we can move them 
         *       into the function. 
         */
        std::map<const MatchedFContractStatement *, std::unique_ptr<SrcWrapperFunction>>
            fContractWrappers;
        std::map<const MatchedLoopInvariant *, std::unique_ptr<SrcWrapperFunction>>
            loopInvWrappers;

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
         * Uses the wrapper generator to create wrapping-functions for all
         * statements in the given contract-block.
         */
        void wrapFunctionContractBlock(
            const MatchedFunctionContract &contract);
            
        /**
         * Uses the wrapper-generator to create wrapping-functions for all
         * loop-invariant-blocks.
         */
        void wrapLoopInvariantBlock(
            const MatchedLoopInvariantBlock &invBlock);

        /**
         * Write the IR-representation of the given module into
         * a file at the given path.
         */
        void writeToFile(const llvm::Module &module, std::string outPath);
    };

}

#endif // PALLAS_SOURCESPECTRANSFORMER_H