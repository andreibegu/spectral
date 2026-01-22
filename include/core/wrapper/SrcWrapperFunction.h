#ifndef PALLAS_SRCWRAPPERFUNCTION_H
#define PALLAS_SRCWRAPPERFUNCTION_H

#include <string>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/DebugInfoMetadata.h>

namespace sourcespec2ir
{

    /**
     * A struct that bundles the name and the string representation of a
     * wrapper-function along with the variables that make up the argument-list.
     * Further contains line-information on where the wrapper should be inserted
     * in the source-file. 
     */
    struct SrcWrapperFunction
    {
        std::string name;
        std::string stringRepr;
        llvm::SmallVector<llvm::DILocalVariable *> argList;
        llvm::SmallVector<std::string> givenArgs;
        llvm::SmallVector<std::string> yieldsArgs;
        size_t insertLoc;
        SrcWrapperFunction(std::string name, std::string stringRepr,
                           llvm::ArrayRef<llvm::DILocalVariable *> argList, 
                           llvm::ArrayRef<std::string> givenArgs,
                           llvm::ArrayRef<std::string> yieldsArgs,
                           size_t insertLoc);
    };
}

#endif // PALLAS_SRCWRAPPERFUNCTION_H