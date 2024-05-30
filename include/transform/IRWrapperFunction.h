#ifndef PALLAS_IRWRAPPERFUNCTION_H
#define PALLAS_IRWRAPPERFUNCTION_H

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <memory>

namespace sourcespec2ir
{

    /**
     * A struct that represents a wrapper-function on the IR-level. 
     * Contains a pointer to the IR-function along with a sequential container that stores pointer 
     * to DILocalVariable which desribe the functions argument list (I.e. which argument corresponds to which 
     * variable on the call-site). 
     */
    struct IRWrapperFunction {
        llvm::Function *function;
        std::unique_ptr<llvm::SmallVector<llvm::DILocalVariable*>> argListDescr;

        IRWrapperFunction(llvm::Function *function, std::unique_ptr<llvm::SmallVector<llvm::DILocalVariable*>> argListDescr);
    };
}

#endif // PALLAS_IRWRAPPERFUNCTION_H