#include <wrapper/IRTypeRepresentation.h>

#include <utility>

#include "PallasException.h"

namespace sourcespec2ir
{

    IRTypeRepresentation::IRTypeRepresentation() : types(),
                                                   attributes()
    {
    }

    void IRTypeRepresentation::addType(llvm::Type *t, llvm::AttributeSet attrs)
    {
        auto idx = types.size();
        types.push_back(t);
        attributes.push_back(attrs);

    }

    llvm::ArrayRef<llvm::Type *> IRTypeRepresentation::getTypes() const 
    {
        return llvm::ArrayRef(types);
    }

    llvm::AttributeSet 
    IRTypeRepresentation::getAttributes(unsigned int idx) const 
    {
        if (idx >= attributes.size())
            throw utils::PallasException("Invalid type-index");

        return attributes[idx];
    }

    size_t IRTypeRepresentation::getNumElems() const 
    {
        return types.size();
    }

    bool IRTypeRepresentation::isReturnByParam() const {
        return types.size() == 1 && types.front()->isVoidTy();
    }

}