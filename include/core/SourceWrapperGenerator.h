#ifndef PALLAS_SOURCEWRAPPERGENERATOR_H
#define PALLAS_SOURCEWRAPPERGENERATOR_H

#include "MatchedSourceSpec.h"
#include "wrapper/SrcLocation.h"
#include "wrapper/SrcWrapperFunction.h"
#include "wrapper/MatchedWrapperFunctions.h"
#include "SpecDeclaration.h"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Instruction.h>

#include <map>
#include <memory>
#include <string>
#include <utility>

namespace sourcespec2ir
{

    /**
     * Abstract class that declares methods which are used to wrap
     * expressions of the source-laguage into functions and compilation-units.
     */
    class SourceWrapperGenerator
    {
    public:
        /**
         * Generates a new wrapper-generator.
         * The provided reference to the specDeclarations must remain valid for
         * the lifetime of the SourceWrapperGenerator.
         */
        SourceWrapperGenerator(
            const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations);

        virtual ~SourceWrapperGenerator() = default;

        /**
         * Generate wrapper-functions for the given specification-constructs.
         */
        std::unique_ptr<MatchedWrapperFunctions> generateWrapperFunctions(
            const MatchedSpecifications &matchedSpecs);

        /**
         * Takes a string that contains a source-language expression along with
         * a sequence of DILocalVariables that describe the source-variables that
         * are accessible in the expression.
         * Returns a string that represents a boolean function in the
         * source-language whose return-value corresponds to the result of the
         * expression.
         * The generated wrapping-function MUST use the same ordering of it's
         * arguments as is defined by parameter srcVars.
         * In case that the wrapper-function is attached to a function
         * (i.e. in the case of a function contract), the returnType-parameter
         * specifices the return-type of this function.
         */
        virtual std::string getWrapperFunctionString(
            const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
            const std::string &expr,
            const std::string &functionName,
            const llvm::DIType *returnType = nullptr) = 0;

        /**
         * For a given SrcWrapperFunction, return the line- and column-offset
         * from the start of the wrapper-function to the beginning of the
         * wrapped expression in the source.
         * This method is used to generate accurate debug-locations for
         * instructions within wrapper-functions.
         *
         * E.g. for a wrapper-function
         * bool w(int a, int b) {
         *     return a >= b;
         * }
         *
         * the result should be <1, 11> because the wrapped expression (a >= b)
         * starts one line and eleven columns from the beginning of the function.
         */
        virtual std::pair<size_t, size_t> getOffsetToExprStart(
            const SrcWrapperFunction &wFun) = 0;

        /**
         * Return the line-number where the wrapper-functions for the
         * function-contract, that is attached to the given, function should be
         * inserted.
         * The default-implementation returns the line of the given function.
         */
        size_t getWrapperInsertLoc(const MatchedFunctionContract &contract);

        /**
         * Return the line-number where the wrapper-function for the
         * loop-invariant, that is attached to the given loop within the given
         * function, should be inserted.
         * The default-implementation returns the line of the parent-function.
         */
        size_t getWrapperInsertLoc(const MatchedLoopInvariantBlock &invBlock);

        /**
         * Return the line-number where the wrapper-function for a given
         * specification-statement should be inserted.
         * The default-implementation returns the line of the parent-function.
         */
        size_t getWrapperInsertLoc(const MatchedSpecStatementBlock &stmntBlock);

        /**
         * Returns a new name for a wrapper-function.
         * The default-implementation simply increments a counter and appends it
         * to 'PALLAS_SPEC_'.
         */
        std::string getNameForWrapperFunction();

        /**
         * Combines the generated wrapper functions and the source-representation
         * of the original program into a source-file.
         *
         * The default-implementation inserts the wrapper-functions in the first
         * blank line above the line-number that is specified in the
         * SrcWrapperFunction.
         * WrapperFunctions with the same specified line-number are inserted in
         * no particular order.
         * Source-language frontends can overwrite this function to implement
         * custom insertion strategies.
         */
        void buildSrcWithWrappers(
            const std::string &inPath, const std::string &outPath,
            llvm::ArrayRef<SrcWrapperFunction *> wrapperFunctions);

        /**
         * Returns a string representation of the given DIType that matches the
         * types representation on the source-level.
         * If the string-representation cannot be generated, an error is raised.
         */
        virtual std::string getSrcReprOfType(const llvm::DIType *type);

        /**
         * Collects the DILocalVariable-nodes that refer to variables
         * that are accessible from the contract of the given function.
         * Global variables are currently not considered.
         */
        virtual std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>>
        getSrcVariablesForFunction(const llvm::Function &fun);

        /**
         * Collects the DILocalVariable-nodes that refer to variables that are
         * accessible in the loop-invariant of the given loop.
         */
        virtual std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>>
        getVarsForLoopInv(llvm::Function *parentFunc,
                          const llvm::BasicBlock *loopHeader);

        /**
         * Collects the DILocalVariable-nodes that refer to variables that can
         * be referenced in a block of specification-statements that is
         * attached to the given instruction
         */
        virtual std::unique_ptr<llvm::SetVector<llvm::DILocalVariable *>>
        getVarsForStatement(llvm::Instruction *inst);

    protected:
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations;

        /**
         * Struct that represents a line of the source-file.
         * Keeps track of the textual represenation along with the line-number
         * in the unmodified source-file.
         */
        struct SrcLine
        {
            std::string content;
            size_t origLine;

            SrcLine(std::string content, size_t origLine)
                : content(content), origLine(origLine)
            {
            }
        };

        /**
         * Adds a const-qualifier to the given base-type.
         */
        virtual std::string addConst(const std::string &baseType);

        /**
         * Adds a pointer-qualifier to the given base-type.
         */
        virtual std::string addPointer(const std::string &baseType);

        /**
         * Adds a namespace-qualifier to the string represnetation of the given
         * parent-namespace.
         */
        virtual std::string addNamespace(const std::string &parentNamespace,
                                         const std::string &childNamespace);

        /**
         * Adds a reference qualifier to the string-representation of the given
         * base-type. Can be overwritten by subclasses.
         * The default-implementation simply throws an error.
         */
        virtual std::string addReference(const std::string &baseType);

        /**
         * Adds a typedef qualifier to the string-representation of the given
         * base-type. Can be overwritten by subclasses.
         * The default-implementation simply throws an error.
         */
        virtual std::string addTypedef(const std::string &baseType,
                                       const std::string &typedefName);

        /**
         * This function is called when a different type-tag is encountered that
         * is not handled by the default addXXX-methods.
         * The default-implementation just throws an error.
         * Subclasses can overwrite this to support other type-tags.
         */
        virtual std::string addOtherQualifier(const std::string &baseType,
                                              const llvm::DIDerivedType &type);

        /**
         * Function is called after the wrapper-functions and declarations
         * have been added to the source.
         * The default-implementation does nothing.
         * Can be overridden by subclasses to apply constomizations.
         */
        virtual void modifySrcWithWrappers(std::vector<SrcLine> &srcLines);

        /**
         * Given a line-number in the original source-file and a vector of lines
         * that represents the modified source-file, this function determines
         * the position where a piece of code (e.g. a wrapper-function) should
         * be inserted.
         * The default-implementation selects the first empty line above the
         * given line number of the source-file.
         */
        std::vector<SrcLine>::iterator findInsertPos(
            const size_t srcLineNumber, std::vector<SrcLine> &srcLines);

        /**
         * Inserts the given string into the vector of lines at the given index.
         * The new lines are inserted before the given index, and a blank-line
         * is added before the content.
         */
        void insertLines(
            std::vector<SrcLine> &lines, const std::string &content,
            std::vector<SrcLine>::iterator currentLine);

    private:
        /* Counter for generating names for the specification functions.
         * TODO: Replace with something more elaborate that takes exisiting
         *       names into account.
         */
        int specFunCnt = 0;

        /**
         * Used internally to recursively build the namespace for a given type
         */
        std::string getSrcNamespace(llvm::DIScope *scope);

        /**
         * Reads the given source-file and converts it into a vector of lines.
         */
        std::unique_ptr<std::vector<SrcLine>> readSrcFile(std::string filePath);

        /**
         * Writes the given vector of lines into a file at the given location.
         */
        void writeSrcFile(const std::string &filePath,
                          const std::vector<SrcLine> &lines);

        /**
         * Creates a wrapping-function for every statement in the
         * given contract-block.
         */
        void wrapFunctionContractBlock(const MatchedFunctionContract &contract,
                                       MatchedWrapperFunctions &matchedWrappers);

        /**
         * Creates a wrapping-function for every statement in the
         * given loop-invariant-block.
         */
        void wrapLoopInvariantBlock(const MatchedLoopInvariantBlock &invBlock,
                                    MatchedWrapperFunctions &matchedWrappers);

        /**
         * Creates a wrapping-function for every statement in the
         * given statement-block.
         */
        void wrapSpecStatementBlock(const MatchedSpecStatementBlock &stmntBlock,
                                    MatchedWrapperFunctions &matchedWrappers);
    };

}

#endif // PALLAS_SOURCEWRAPPERGENERATOR_H