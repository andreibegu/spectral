#ifndef PALLAS_SOURCESPECTRANSFORMER_H
#define PALLAS_SOURCESPECTRANSFORMER_H

#include <string>
#include <fstream>
#include <map>
#include <memory>

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

#include "SpecParser.h"

#include "SourceSpecMatcher.h"
#include "SourceCompilerDriver.h"
#include "SourceWrapperGenerator.h"
#include "SpecUnifier.h"
#include "OutputConfig.h"
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
            const utils::OutputConfig &outputConfig,
            const std::string &workingDir, 
            bool runMem2Reg);

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
         * 
         * The given path refers to the original source-file from which the 
         * parse-tree of the specifications was created. 
         */
        std::unique_ptr<SourceSpecMatcher> createSpecMatcher(
            SpecParser::FileContext *specParseTree, llvm::Module *irModule, 
            std::string srcFilePath);

    private:
        std::string srcInPath;
        std::string outPath;
        std::string workingDir;
        const utils::OutputConfig outputConfig;
        const bool runMem2Reg;

        std::unique_ptr<SourceWrapperGenerator> wrapperGenerator;

        std::unique_ptr<SourceCompilerDriver> srcCompilerDriver;

        std::unique_ptr<SpecUnifier> specUnifier;

        // Pointer to the LLVM context
        std::unique_ptr<llvm::LLVMContext> irContext;

        // Pointer to the loaded llvm-ir module
        std::unique_ptr<llvm::Module> irModule;

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
         * Maps that relate the matched specifications to the corresponding
         * wrapper-functions.
         */
        std::map<const MatchedFContractStatement *, std::unique_ptr<SrcWrapperFunction>>
            fContractWrappers;
        std::map<const MatchedLoopInvariant *, std::unique_ptr<SrcWrapperFunction>>
            loopInvWrappers;
        std::map<const MatchedSpecStatement *, std::unique_ptr<SrcWrapperFunction>>
            statementWrappers;

        /**
         * Runs the mem2reg-pass on the given module.
         */
        void promoteMemToReg(llvm::Module &module);

        /**
         * Runs the verify-pass in the given module and throws an exception
         * if the module is not wellformed.
         */
        void verifyModule(llvm::Module &module, const std::string msg);

        /**
         * Encodes a block of function-contract statements into metadata
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
         * Encodes a block of loop-invariants into metadata.
         */
        void encodeLoopInvBlock(const MatchedLoopInvariantBlock &invBlock);

        /**
         * Encodes a single loop-invariant into a metadata-node.
         * The new Metadata node is added to the back of the given vector.
         */
        void encodeLoopInv(const MatchedLoopInvariant &inv,
                           llvm::SmallVector<llvm::Metadata *> &nodes);

        /**
         * Encodes a block of specification-statements into metadata.
         */
        void encodeStmntBlock(const MatchedSpecStatementBlock &stmntBlock);

        /**
         * Encodes a statement from a block of spec-statements into a md-node.
         * Returns a pointer to the new MD-node. Ownership of the new node
         * lies with the llvm-context.
         */
        llvm::MDNode *encodeSpecStatement(const MatchedSpecStatement &stmnt);

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
         * Uses the wrapper-generator to create wrapping-functions for all
         * blocks of specification-statements.
         */
        void wrapSpecStatementBlock(
            const MatchedSpecStatementBlock &stmntBlock);

        /**
         * Modifies the debug-locations of the instructions in the generated
         * wrapper-functions to represent the locations from the original
         * source-file.
         * Modifies irModule and uses the passed matched specifications 
         * to find the wrapper-functions and to determine the appropriate scopes.
         */
        void rewriteWrapperLocations(const MatchedSpecifications &matchedSpecs);

        /**
         * Modifies the debug-locations of the instructions in the passed
         * wrapper function to represent the location in the source file.
         * 
         * The passed compilation-unit and scope-file are used as the 
         * cu and scope of the wrapper-function.
         */
        void rewriteWrapperLocations(
            const SrcWrapperFunction &wFun, const SrcLocation &exprStartLoc,
            llvm::DICompileUnit *cu, llvm::DIFile *scopeFile, 
            llvm::DIScope *scope);

        /**
         * Write the IR-representation of the given module into
         * a file at the given path.
         */
        void writeToFile(const llvm::Module &module, std::string outPath);

        /**
         * Removes duplicate definitions of globals from the second module by
         * turning all global definitions in 'moduleWithWrappers' that also
         * occur in 'origModule' into declarations.
         * As a result, 'moduleWithWrappers' can be linked
         * into 'origModule' without conflicts.
         */
        void removeDuplicateDefinitions(const llvm::Module &origModule,
                                        llvm::Module &moduleWithWrappers);

        /**
         * Adds the functions that belong to the given set of wrapper-functions 
         * to the module's llvm.used-list.
         */
        void markWrapperFunctionsAsUsed(llvm::Module &module,
            llvm::ArrayRef<SrcWrapperFunction *> wrappers);

        /**
         * Generates a MDTuple that encodes a source-location.
         */
        llvm::MDTuple *encodeSrcLoc(const SrcRange &srcLoc);

        /**
         * Helper that returns the DIFile for a given function and checks for 
         * nullpointers. 
         */
        llvm::DIFile *getFileForFunc(llvm::Function *f);

        /**
         * Helper that returns the DIScope for a given function and checks for 
         * nullpointers. 
         */
        llvm::DIScope *getScopeForFunc(llvm::Function *f);

        /**
         * Helper that returns the cimpilation-unit for a given function and 
         * checks for nullpointers. 
         */
        llvm::DICompileUnit *getCUForFunc(llvm::Function *f);

        

        /**
         * Returns the path where the source-file that has been extended with
         * wrappers should be saved.
         */
        std::string getPathForSourceWithWrappers();
    };

}

#endif // PALLAS_SOURCESPECTRANSFORMER_H