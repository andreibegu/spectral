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
         * Given a string that represents the definition of a ghost-argument,
         * extract the type of the argument.
         * 
         * The default-implementation assumes a C-style syntax of the shape 
         * TYPE NAME
         */
        virtual SrcTypeRepresentation getTypeForGhostArgDef(
            llvm::StringRef argDef);
    };

}

#endif // PALLAS_SOURCESYNTAXRESOLVER_H