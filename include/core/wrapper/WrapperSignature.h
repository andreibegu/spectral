#ifndef PALLAS_WRAPPERSIGNATURE_H
#define PALLAS_WRAPPERSIGNATURE_H

#include "wrapper/SrcTypeRepresentation.h"

#include <llvm/ADT/SmallVector.h>
#include <optional>
#include <string>

namespace sourcespec2ir
{

    struct WrapperSignature
    {
        /**
         * String-representation of the wrapper-function's arguments.
         */
        llvm::SmallVector<std::string> args;

        /**
         * If the wrapper does not have a boolean return type, this contains
         * string representation of the wrappers return type.
         * TODO: Refactor the language specific syntax into a class and make
         *         this a pure string.
         */
        std::optional<SrcTypeRepresentation> returnTy;

        /**
         * If this wrapper is used in the context of a function contract, this
         * referes to the return-type of the associated parent-function.
         */
        std::optional<std::string> parentRetTy;

        inline void addArg(const std::string &arg) { args.push_back(arg); }
    };

}

#endif // PALLAS_WRAPPERSIGNATURE_H