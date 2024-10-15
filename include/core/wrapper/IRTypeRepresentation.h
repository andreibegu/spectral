#ifndef PALLAS_IRTYPEREPRESENTATION_H
#define PALLAS_IRTYPEREPRESENTATION_H

#include <memory>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/Type.h>

namespace sourcespec2ir
{

    /**
     * A struct that represents how a type from the source-language is
     * represented on the IR-level when it is passed to, or returned from,
     * a function.
     *
     * Required because different compilers represent e.g. small structs differently
     * when passing them to a function.
     * 
     * When a return-type is represented, two cases are possible:
     * - If the type-vector only contains a void-type, this indicates that the 
     *   return-value is handles through an sret-parameter. 
     *   In this case, the attributes belong to the sret-parameter. 
     * - If the type-vector contains a single, non-void type, this indicates
     *   that the function retunrs its result normally. In this case, 
     *   the attributes belong to the return-type.
     */
    struct IRTypeRepresentation
    {
        private:
        llvm::SmallVector<llvm::Type *> types;
        llvm::SmallVector<llvm::AttributeSet> attributes;

        public:
        /**
         * Creates an empty type-representation.
         */
        IRTypeRepresentation();

        /**
         * Adds the given type to the back of the type-representation and 
         * associates it with the given set of attributes. 
         */
        void addType(llvm::Type *t, llvm::AttributeSet attrs);

        /**
         * Returns a reference to the types of the representation.
         */
        llvm::ArrayRef<llvm::Type *> getTypes() const;

        /**
         * Return reference to the attributes for the type at the given index.
         */
        llvm::AttributeSet getAttributes(unsigned int idx) const;

        /**
         * Returns the number of elements in the type-representation.
         */
        size_t getNumElems() const;

        /**
         * If this ype-representation represents a return-value, 
         * check if it represents a return by sret-parameter.
         */
        bool isReturnByParam() const;
    };
}

#endif // PALLAS_IRTYPEREPRESENTATION_H