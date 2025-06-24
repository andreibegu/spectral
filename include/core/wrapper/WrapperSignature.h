#ifndef PALLAS_WRAPPERSIGNATURE_H
#define PALLAS_WRAPPERSIGNATURE_H

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
         * If this wrapper is used in the context of a function contract, this
         * refere to the return-type of the associated parent-function.  
         */
        std::optional<std::string> parentRetTy;

        inline void addArg(const std::string &arg) { args.push_back(arg); }
    };

}

#endif // PALLAS_WRAPPERSIGNATURE_H