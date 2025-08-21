#ifndef PALLAS_IRTYPEREPRESENTATION_H
#define PALLAS_IRTYPEREPRESENTATION_H

#include <memory>
#include <string>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/Instructions.h>
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
        llvm::SmallVector<llvm::Type *, 4> types;
        llvm::SmallVector<llvm::AttributeSet, 4> attributes;

    public:
        /**
         * Constructs an IRTypeRepresentation for a function argument
         * based on the specified arguments of the given function-call
         * by copying the types and attributes.
         * - startIdx specifies the index of the first argument that should be
         *   used for the representation
         * - numElem specified how many arguments belong to the type-representation.
         */
        static std::unique_ptr<IRTypeRepresentation> getArgReprFromCall(
            const llvm::CallInst &call, unsigned int startIdx, unsigned int numElem);

        /**
         * Constructs an IRTypeRepresentation that represents the return
         * of the given function-call.
         */
        static std::unique_ptr<IRTypeRepresentation> getRetReprFromCall(
            const llvm::CallInst &call);

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

        /**
         * Return a string-representation of this IRTypeRepresentation. 
         * E.g. used in the naming of functions from the specification-library.
         */
        std::string asString() const;

        /**
         * Equality-operator
         */
        bool operator==(const IRTypeRepresentation &other)
        {
            // Check if number of types it the same
            auto sThis = types.size();
            auto sOther = other.types.size();
            if (sThis != sOther)
                return false;

            // Compare the type-vectors & attributes
            for (unsigned int i = 0; i < sThis; ++i)
                if (types[i] != other.types[i] ||
                    attributes[i] != other.attributes[i])
                    return false;

           return true;
        }
    };
}

#endif // PALLAS_IRTYPEREPRESENTATION_H