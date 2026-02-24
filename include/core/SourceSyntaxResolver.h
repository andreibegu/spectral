#ifndef PALLAS_SOURCESYNTAXRESOLVER_H
#define PALLAS_SOURCESYNTAXRESOLVER_H

#include "wrapper/SrcTypeRepresentation.h"

#include <llvm/ADT/StringRef.h>

namespace sourcespec2ir
{

    /**
     * Abstract class that declares methods for handling source language
     * specific syntax.
     * This class is expected to be stateless.
     */
    class SourceSyntaxResolver
    {
    public:
        SourceSyntaxResolver();

        virtual ~SourceSyntaxResolver() = default;

        /**
         * Given a string containing the definition of a function argument,
         * return the name of the argument.
         * The default implementation assumes C-style argument definitions.
         * E.g. for 'int a' this would return 'a'.
         * Frontends might have to override this to match it to the languages
         * specific syntax.
         */
        virtual std::string getArgName(llvm::StringRef argDef);

        /**
         * Given a string that represents the definition of a ghost-argument,
         * extract the type of the argument.
         *
         * The default-implementation assumes a C-style syntax of the shape
         * TYPE NAME
         */
        virtual SrcTypeRepresentation getTypeForGhostArgDef(
            llvm::StringRef argDef);

        /**
         * Give the string-representation of a ghost method, returns the name
         * of the ghost method as a string.
         * The default-implementation looks for the first occurence of '(' and
         * returns the whitespace free character sequence in front of it.
         * If the impl. does not contain a '(' character, an empty string is
         * returned.
         * I.e. for 'int foo(...' it would return 'foo'.
         *
         * Can be overridden if the default-implementation is not suitable.
         */
        virtual std::string getGhostMethodName(llvm::StringRef impl);

        /**
         * Given a string that represents a language-specific definition of a
         * ghost method or function, return the arguments of the method.
         * The default implementation assumes a C-style syntax
         * (i.e. function arguments are enclosed in '(' and ')' and separated
         * by ',').
         * E.g. for 'int foo(int a, int b) { ...' it would return
         * 'int a' and 'int b'.
         *
         * If the default implementation is not suitable for a particular
         * language, it can be overridden.
         */
        virtual llvm::SmallVector<std::string> getGhostMethodArgs(
            llvm::StringRef impl);

        /**
         * Given a string that represents a language-specific definition of a
         * ghost method / function, return the method's return type as a string.
         * The default implementation assumes a C-style syntax
         *
         * If the default implementation is not suitable for a particular
         * language, it can be overridden.
         */
        virtual std::string getGhostMethodRetTy(llvm::StringRef impl);
    };

}

#endif // PALLAS_SOURCESYNTAXRESOLVER_H