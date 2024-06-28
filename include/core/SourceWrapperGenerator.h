#ifndef PALLAS_SOURCEWRAPPERGENERATOR_H
#define PALLAS_SOURCEWRAPPERGENERATOR_H

#include "wrapper/SrcWrapperFunction.h"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/DebugInfoMetadata.h>

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
         * Takes a string that contains a source-language expression along with a
         * sequence of DILocalVariables that describe the source-variables that
         * are accessible in the expression.
         * The generated wrapping-function MUST use the same ordering of it's
         * arguments as is defined by parameter srcVars.
         */
        virtual std::unique_ptr<SrcWrapperFunction> getWrappingFunction(
            llvm::ArrayRef<llvm::DILocalVariable *> srcVars, const std::string &expr) = 0;

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
        virtual std::string getSrcReprOfType(const llvm::DIType *type) = 0;
    };

}

#endif // PALLAS_SOURCEWRAPPERGENERATOR_H