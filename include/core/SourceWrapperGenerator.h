#ifndef PALLAS_SOURCEWRAPPERGENERATOR_H
#define PALLAS_SOURCEWRAPPERGENERATOR_H

#include "MatchedSourceSpec.h"
#include "wrapper/SrcWrapperFunction.h"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/DebugInfoMetadata.h>

#include <map>
#include <memory>
#include <string>

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
         * Takes a string that contains a source-language expression along with
         * a sequence of DILocalVariables that describe the source-variables that
         * are accessible in the expression.
         * Returns a string that represents a boolean function in the
         * source-language whose return-value corresponds to the result of the
         * expression.
         * The generated wrapping-function MUST use the same ordering of it's
         * arguments as is defined by parameter srcVars.
         */
        virtual std::string getWrapperFunctionString(
            const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
            const std::string &expr,
            const std::string &functionName) = 0;

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
            const llvm::ArrayRef<SrcWrapperFunction *> &wrapperFunctions);

        /**
         * Takes a string-representation of a source-language function and turns
         * it into a full compilation unit in the source language.
         *
         * Also receives a set of declarations that have to be included in the
         * generated compilation unit (e.g. types from the spec-library, ...).
         * These are the declarations from the optional peclaration block
         * in the specifications.
         *
         * E.g. adds imports for the specification library, ...
         */
        virtual std::string assembleSrcCompilationUnit(
            const SrcWrapperFunction &wrapperFunction,
            const llvm::ArrayRef<std::string> &declarations) = 0;

        /**
         * Returns a string representation of the given DIType that matches the
         * types representation on the source-level.
         * If the string-representation cannot be generated, an error is raised.
         */
        std::string getSrcReprOfType(const llvm::DIType *type);

    protected:
        /**
         * Adds a const-qualifier to the given base-type.
         */
        virtual std::string addConst(const std::string &baseType) = 0;

        /**
         * Adds a pointer-qualifier to the given base-type.
         */
        virtual std::string addPointer(const std::string &baseType) = 0;

        /**
         * Adds a namespace-qualifier to the string represnetation of the given
         * parent-namespace.
         */
        virtual std::string addNamespace(const std::string &parentNamespace,
                                         const std::string &childNamespace) = 0;

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
         * Inserts the given string into the vector of lines at the given index. 
         * The new lines are inserted before the given index, and a blank-line 
         * is added before the content. 
         */
        void insertLines(
            std::vector<SrcLine> &lines, const std::string &content, 
            std::vector<SrcLine>::iterator currentLine);
    };

}

#endif // PALLAS_SOURCEWRAPPERGENERATOR_H