#ifndef PALLAS_SOURCEWRAPPERGENERATOR_H
#define PALLAS_SOURCEWRAPPERGENERATOR_H

#include "MatchedSourceSpec.h"
#include "wrapper/SrcLocation.h"
#include "wrapper/SrcTypeRepresentation.h"
#include "wrapper/SrcWrapperFunction.h"
#include "wrapper/MatchedWrapperFunctions.h"
#include "wrapper/WrapperLocMarker.h"
#include "wrapper/WrapperSignature.h"
#include "SpecDeclaration.h"
#include "SpecPredicate.h"
#include "SourceSyntaxResolver.h"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Instruction.h>

#include <list>
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
         * The provided references to the specDeclarations and InsertLocations
         * must remain valid for the lifetime of the WrapperGenerator.
         */
        SourceWrapperGenerator(
            const std::string srcInFile,
            const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations,
            const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers, 
            std::unique_ptr<SourceSyntaxResolver> syntaxResolver);

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
            const WrapperSignature &sig,
            const std::string &expr,
            const std::string &functionName) = 0;

        /**
         * Generates a string representation for a given predicate definition.
         * The function must return a boolean.
         * Argument 'sig' contains the signature and name of the function.
         * Argument 'body' contains the expression that makes up the predicate.
         *
         * E.g. for C a call with the following inputs
         * sig:  'hasVal(int *ptr, int val)'
         * body: '_sep(_Perm(ptr, _writePerm),
         *             *ptr == val)'
         * could be represented with the following function:
         * '
         * bool hasVal(int *ptr, int val) {
         *      return _sep(_Perm(ptr, _writePerm),
         *             *ptr == val);
         * }
         * '
         */
        virtual std::string getPredicateDefString(const std::string &name,
                                                  const std::string &args,
                                                  const std::string &body) = 0;

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
         * Analogous to getOffsetToExprStart for wrapper fucntions, but for
         * predicate definitions.
         */
        virtual std::pair<size_t, size_t> getOffsetToExprStart(
            const SpecPredicate &pDef) = 0;

        /**
         * Return the line-number where the wrapper-functions for the
         * function-contract, that is attached to the given, function should be
         * inserted.
         * The default-implementation returns the line of the given function.
         */
        virtual size_t getWrapperInsertLoc(const MatchedFunctionContract &contract);

        /**
         * Return the line-number where the wrapper-functions for the
         * given ghost contract should be inserted.
         * The default-implementation returns the line of the
         * contract definition.
         */
        virtual size_t getWrapperInsertLoc(
            const MatchedGhostContract &contract);

        /**
         * Return the line-number where the wrapper-function for the
         * loop-invariant, that is attached to the given loop within the given
         * function, should be inserted.
         * The default-implementation returns the line of the parent-function.
         */
        virtual size_t getWrapperInsertLoc(
            const MatchedLoopInvariantBlock &invBlock);

        /**
         * Return the line-number where the wrapper-function for a given
         * specification-statement should be inserted.
         * The default-implementation returns the line of the parent-function.
         */
        virtual size_t getWrapperInsertLoc(
            const MatchedSpecStatementBlock &stmntBlock);

        /**
         * Return the line number where the wrapper functions for a block of
         * bindings to given-arguments should be inserted.
         * The default implementation returns the line of the parent-function.
         */
        virtual size_t getWrapperInsertLoc(
            const MatchedGhostArgBindingBlock &bindingBlock);

        /**
         * Return the line-number where the wrapper-functions for a given
         * contract that is mapped to an external or generated function
         * should be inserted.
         */
        virtual size_t getExternalWrapperInsertLoc(
            const MatchedFunctionContract &contract);

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
            llvm::ArrayRef<SrcWrapperFunction *> wrapperFunctions,
            llvm::ArrayRef<SpecPredicate> predicates,
            llvm::ArrayRef<GhostMethod *> ghostMethods);

        /**
         * Returns a textual representation of the given DIType that matches the
         * types representation on the source-level.
         * If the representation cannot be generated, an error is raised.
         */
        virtual SrcTypeRepresentation getSrcReprOfType(const llvm::DIType *type);

        /**
         * Collects the DILocalVariable-nodes that refer to variables
         * that are accessible from the contract of the given function.
         * Global variables are currently not considered.
         * The result is sorted based on the arg-idx of the DIVariables.
         */
        virtual llvm::SmallVector<llvm::DILocalVariable *>
        getSrcVariablesForFunction(const llvm::Function &fun);

        /**
         * Collects the DILocalVariable-nodes that refer to variables that are
         * accessible in the loop-invariant of the given loop.
         */
        virtual llvm::SetVector<llvm::DILocalVariable *>
        getVarsForLoopInv(llvm::Function *parentFunc,
                          const llvm::BasicBlock *loopHeader);

        /**
         * Collects the DILocalVariable-nodes that refer to variables that can
         * be referenced in a block of specification-statements that is
         * attached to the given instruction
         */
        virtual llvm::SetVector<llvm::DILocalVariable *>
        getVarsForStatement(llvm::Instruction *inst);

    protected:
        const std::string srcInFile;
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations;
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers;

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
         * Return an appropriate SyntaxResolver for the used language.
         */
        SourceSyntaxResolver *getSyntaxResolver();

        /**
         * Build a wrapper-signature based on the given DILocalVariables that
         * represent the arguments, the return type of the wrapper and the
         * DIType that represents the return-type` of the parent function.
         * Frontends can override this to influence the construction of the
         * wrapper-function's signature.
         */
        virtual WrapperSignature getWrapperSignature(
            const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
            const std::optional<SrcTypeRepresentation> &retType,
            const llvm::DIType *parentRetTy = nullptr);

        /**
         * Build a string-representation of an argument from a string of the
         * arguments type and name.
         */
        virtual std::string buildArgStr(const SrcTypeRepresentation &type,
                                        const std::string &name);

        /**
         * Constructs a string representation for a DICompositeType with the
         * DW_TAG_array_type-tag.
         * Can be overwritten by subclasses if the default-implementation
         * is not sufficient.
         */
        virtual SrcTypeRepresentation getSrcReprOfArrType(
            const llvm::DICompositeType *type);

        /**
         * Gets the SrcTypeRepresentation of the `void` type.
         */
        virtual SrcTypeRepresentation getVoid();

        /**
         * Adds a const-qualifier to the given base-type.
         */
        virtual void addConst(SrcTypeRepresentation &baseType);

        /**
         * Adds a pointer-qualifier to the given base-type.
         */
        virtual void addPointer(SrcTypeRepresentation &baseType);

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
        virtual void addReference(SrcTypeRepresentation &baseType);

        /**
         * Adds a typedef qualifier to the string-representation of the given
         * base-type. Can be overwritten by subclasses.
         * The default-implementation simply throws an error.
         */
        virtual void addTypedef(SrcTypeRepresentation &baseType,
                                const std::string &typedefName);

        /**
         * This function is called when a different type-tag is encountered that
         * is not handled by the default addXXX-methods.
         * The default-implementation just throws an error.
         * Subclasses can overwrite this to support other type-tags.
         */
        virtual void addOtherQualifier(SrcTypeRepresentation &baseType,
                                       const llvm::DIDerivedType &type);

        /**
         * This function is called to add a dimention to an array-type.
         * Can be overwritten by subclasses.
         * The default-implementation simply throws an error.
         */
        virtual void addArrayDim(SrcTypeRepresentation &baseType,
                                 const std::string &dimSize);

        /**
         * Function is called after the wrapper-functions and declarations
         * have been added to the source.
         * The default-implementation does nothing.
         * Can be overridden by subclasses to apply constomizations.
         */
        virtual void modifySrcWithWrappers(std::list<SrcLine> &srcLines);

        /**
         * Given a line-number in the original source-file and a list of lines
         * that represents the modified source-file, this function determines
         * the position where a piece of code (e.g. a wrapper-function) should
         * be inserted.
         * The default-implementation selects the first empty line above the
         * given line number of the source-file.
         */
        std::list<SrcLine>::iterator findInsertPos(
            const size_t srcLineNumber, std::list<SrcLine> &srcLines);

        /**
         * Inserts the given string into the vector of lines at the given index.
         * The new lines are inserted before the given index, and a blank-line
         * is added before the content.
         */
        void insertLines(
            std::list<SrcLine> &lines, const std::string &content,
            std::list<SrcLine>::iterator currentLine);

    private:
        /* Counter for generating names for the specification functions.
         * TODO: Replace with something more elaborate that takes exisiting
         *       names into account.
         */
        int specFunCnt = 0;

        std::unique_ptr<SourceSyntaxResolver> syntaxResolver;

        /**
         * Used internally to recursively build the namespace for a given type
         */
        std::string getSrcNamespace(llvm::DIScope *scope);

        /**
         * Reads the given source-file and converts it into a vector of lines.
         */
        std::unique_ptr<std::list<SrcLine>> readSrcFile(std::string filePath);

        /**
         * Writes the given vector of lines into a file at the given location.
         */
        void writeSrcFile(const std::string &filePath,
                          const std::list<SrcLine> &lines);

        /**
         * Creates a wrapping-function for every statement in the
         * given contract-block.
         */
        void wrapFunctionContractBlock(
            const MatchedFunctionContract &contract,
            MatchedWrapperFunctions &matchedWrappers);

        /**
         * Creates a wrapping-function for every statement in the
         * given ghost contract.
         */
        void wrapGhostContract(
            const MatchedGhostContract &contract,
            MatchedWrapperFunctions &matchedWrappers);

        /**
         * Creates a wrapping-function for every statement in the
         * given loop-invariant-block.
         */
        void wrapLoopInvariantBlock(
            const MatchedLoopInvariantBlock &invBlock,
            const MatchedFunctionContract *parentContract,
            MatchedWrapperFunctions &matchedWrappers);

        /**
         * Creates a wrapping-function for every statement in the
         * given statement-block.
         */
        void wrapSpecStatementBlock(
            const MatchedSpecStatementBlock &stmntBlock,
            const MatchedFunctionContract *parentContract,
            MatchedWrapperFunctions &matchedWrappers);

        void wrapGivenArgBlock(
            const MatchedGhostArgBindingBlock &bindingBlock,
            const MatchedFunctionContract *parentContract,
            const MatchedFunctionContract *calledContract,
            MatchedWrapperFunctions &matchedWrappers);

        /**
         * Adds the ghost arguments from the given contract to the given
         * wrapper signature.
         */
        void addGhostArgsToSig(WrapperSignature &sig,
                               const MatchedFunctionContract &contr,
                               bool skipYields = false);

        /**
         * Extracts the names of the given and yields arguments and adds them
         * to the corresponding vectors.
         * If contr is null, nothing happens.
         */
        void addGhostArgNames(const MatchedFunctionContract *contr,
                              llvm::SmallVector<std::string> &givenArgs,
                              llvm::SmallVector<std::string> &yieldsArgs);
    };

}

#endif // PALLAS_SOURCEWRAPPERGENERATOR_H