#ifndef PALLAS_SRCWRAPPERFUNCTION_H
#define PALLAS_SRCWRAPPERFUNCTION_H

#include <string>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/DebugInfoMetadata.h>

namespace sourcespec2ir
{

    /**
     * A struct that bundles the name and the string representation of a
     * wrapper-function along with the variables that make up the argument-list.
     */
    struct SrcWrapperFunction
    {
        std::string name;
        std::string stringRepr;
        llvm::ArrayRef<llvm::DILocalVariable *> argList;
        SrcWrapperFunction(std::string name, std::string stringRepr,
                           const llvm::ArrayRef<llvm::DILocalVariable *> &argList);
    };
}

#endif // PALLAS_SRCWRAPPERFUNCTION_H