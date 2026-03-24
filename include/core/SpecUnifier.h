#ifndef PALLAS_SPECUNIFIER_H
#define PALLAS_SPECUNIFIER_H

#include "wrapper/IRTypeRepresentation.h"

#include <vector>
#include <tuple>
#include <optional>
#include <utility>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

namespace sourcespec2ir
{

    /**
     * Class that converts the IR-level specifications from the source-language
     * specific format to the unified specification format of Pallas.
     * This involves replacing the calls to source-language specific
     * specification-functions to calls with the unified
     * pallas-specification-functions.
     *
     * Does not take ownership of the passed module or context.
     */
    class SpecUnifier
    {
    public:
        /**
         * Enum to identify the different constructs of the specification library
         */
        enum SpecLibElement
        {
            Imply,          // Implication
            ScAnd,          // Short-circuting and
            ScOr,           // Short-circuting or
            SepConj,        // Separating conjunction
            Old,            // Old value of variable
            Result,         // Result in fucntion contract
            BoundVar,       // Bound variable of a quantifier
            Exists,         // Existential quantifier
            Forall,         // Universal quantifier
            ForallSep,      // Universal quantifier with separating conjunction
            FracOf,         // Construct fraction from two ints
            Perm,           // Assert permission over memory location
            PtrBlockLength, // Length pointer-block
            PtrBlockOffset, // Offset in pointer block
            PtrLength,      // Size of valid locations after the given pointer
            Unfolding,      // Construct for temporarily unfolding a predicate
            SeqNew,         // New sequence
            SeqSize,        // Size of sequence
            SeqGet,         // Get element from sequence
            SeqPrepend,     // Prepend element to sequence
            SeqEquals,      // Equality of two sequences
            SeqSlice,       // Slice of a sequence
            SeqSet          // Update value of sequence
        };

        SpecUnifier(const bool printDebugInfo);

        virtual ~SpecUnifier() = default;

        /**
         * Invokes the specification-unification on the given module.
         */
        void unifySpecifications(llvm::Module *irModule, llvm::LLVMContext *irContext);

    protected:
        bool printDebugInfo;

        // Pointers to the loaded llvm-ir module and context
        llvm::Module *irModule;
        llvm::LLVMContext *irContext;

        /*
         * Functions that contain source-language specific behavior.
         */

        /**
         * If the given function is from the source specification-library,
         * returns the corresponding value from the SpecLibElement-enum.
         * Otherwise, an empty optional is returned.
         */
        virtual std::optional<SpecLibElement> isSrcSpecificationFunction(
            const llvm::Function &f) = 0;

        /**
         * Return a pointer to the type that is used by the source
         * specification-library to represent fractions.
         */
        virtual llvm::Type *getSrcLibFracType() = 0;

        /**
         * Checks if the given type represents a sequence in the source
         * specification-library.
         */
        virtual bool isSrcSeqType(const llvm::StructType &t) = 0;

        /**
         * Takes a type that represents an ADT from the source
         * specification-library and returns the content-type of this ADT.
         */
        virtual llvm::Type *getContentTypeOfADTTy(const llvm::Type &t) = 0;

        /*
         * Functions that are used to transform a specific call to the source
         * specification-library into a call to the unified specification library.
         *
         * The default implementation of these functions performs a 1:1 mapping
         * of the source-language specific call to a call to the unified
         * specification function (i.e. the called function is replaced and the
         * arguments of the old call are used as arguments for the new call
         * (in the same order) and the attributes that are required by the
         * unified sepc-ob are added).
         *
         * In cases where a more elaborate transformation is required, the
         * source-language specific specialization of the SpecUnifier can
         * override these functions to implement a custom unification-startegy,
         */
        virtual llvm::CallInst *transformImply(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformScAnd(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformScOr(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformSepConj(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformOld(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformResult(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformBoundVar(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformExists(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformForall(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformForallSep(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformFracOf(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformPerm(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformPtrBlockLength(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformPtrBlockOffset(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformPtrLength(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformUnfolding(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformSeqNew(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformSeqSize(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformSeqGet(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformSeqPrepend(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformSeqEquals(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformSeqSlice(llvm::CallInst &oldCall);

        virtual llvm::CallInst *transformSeqSet(llvm::CallInst &oldCall);

        /*
         *  General helper functions for subclasses
         */

        /**
         * Takes a value that represents an ADT from the source
         * specification-library and returns the content-type of this ADT.
         */
        llvm::Type *getContentTypeOfADT(const llvm::Value &v);

        /*
         * Functions to generate and get functions of the unified
         * specification-library.
         */

        /**
         * Returns pointer to the unified specification-function for implications.
         */
        llvm::Function *getUSpecImply();

        /**
         * Returns pointer to the unified specification-function for
         * the short-circuting and-operator.
         */
        llvm::Function *getUSpecScAnd();

        /**
         * Returns pointer to the unified specification-function for
         * the short-circuting or-operator.
         */
        llvm::Function *getUSpecScOr();

        /**
         * Returns pointer to the unified specification-function for
         * the separating conjunction.
         */
        llvm::Function *getUSpecSepConj();

        /**
         * Returns pointer to the unified specification-function for the
         * \old-construct of the given type.
         */
        llvm::Function *getUSpecOld(const IRTypeRepresentation &argRepr,
                                    const IRTypeRepresentation &retRepr);

        /**
         * Returns pointer to the unified specification-function for the
         * \result-construct of the given type.
         */
        llvm::Function *getUSpecResult(const IRTypeRepresentation &retRepr);

        /**
         * Returns pointer to the specification-function for the bound-variables
         * construct of the given type.
         */
        llvm::Function *getUSpecBoundVar(const IRTypeRepresentation &retRepr);

        /**
         * Returns pointer to the unified specification-function for the
         * existential quantifier.
         */
        llvm::Function *getUSpecExists();

        /**
         * Returns pointer to the unified specification-function for the
         * universal quantifier.
         */
        llvm::Function *getUSpecForall();

        /**
         * Returns pointer to the unified specification-function for the
         * separating universal quantifier.
         */
        llvm::Function *getUSpecForallSep();

        /**
         * Returns pointer to the unified specification-function for creating
         * a fraction.
         */
        llvm::Function *getUSpecFracOf();

        /**
         * Return pointer to the unified specification-function for specifying
         * permission over a location.
         */
        llvm::Function *getUSpecPerm();

        /**
         * Return pointer to the unified specification-functions
         * regarding validity of pointers
         */
        llvm::Function *getUSpecPtrBlockLength();
        llvm::Function *getUSpecPtrBlockOffset();
        llvm::Function *getUSpecPtrLength();

        /**
         * Returns a pointer to the unified specification-function for the
         * 'unfolding' construct for predicates.
         */
        llvm::Function *getUSpecUnfolding(const IRTypeRepresentation &retRepr,
                                          const IRTypeRepresentation &argRepr);

        /**
         * Returns pointer to the unified specification-function for creating
         * empty sequences with the given content type.
         */
        llvm::Function *getUSpecSeqNew(llvm::Type *type);

        /**
         * Returns a pointer to the unified specification-function for getting
         * the size of a sequence with the given content-type.
         */
        llvm::Function *getUSpecSeqSize(llvm::Type *type);

        /**
         * Returns a pointer to the unified specification-function for accessing
         * an element of a sequence with the given content-type.
         * The representation of the returned element is specified through the
         * provided type-representation.
         */
        llvm::Function *getUSpecSeqGet(llvm::Type *type,
                                       const IRTypeRepresentation &retRepr);

        /**
         * Returns a pointer to the unified specification-function for
         * prepending an element to a sequence with the given content type.
         * The representation of the prepended element is specified through
         * the passed IRTypeRepresentation.
         */
        llvm::Function *getUSpecSeqPrepend(llvm::Type *type,
                                           const IRTypeRepresentation &argRepr);

        /**
         * Returns a pointer to the unified specification-function for
         * the equality-operator for sequences with the given content type.
         */
        llvm::Function *getUSpecSeqEquals(llvm::Type *type);

        /**
         * Returns a pointer to the unified specification-function for
         * getting a slice of a sequence with the given content type.
         */
        llvm::Function *getUSpecSeqSlice(llvm::Type *type);

        /**
         * Returns a pointer to the unified specification-function for
         * updating an element of a sequence with the given content type.
         * The representation of the updated element is specified through
         * the passed IRTypeRepresentation.
         */
        llvm::Function *getUSpecSeqSet(llvm::Type *type,
                                       const IRTypeRepresentation &argRepr);

        /**
         * Returns pointer to the type that is used to represent fractions
         * in the unified specification representation.
         */
        llvm::StructType *getUSpecFracType();

        /**
         * Returns pointer to the type that is used to represent sequences
         * with the given content-type in the unified specification representation.
         */
        llvm::StructType *getUSpecSeqType(llvm::Type *type);

        /**
         * Helper-functions that are used in the implementation of the spec-unifier
         * and can be used by subclasses.
         */

        /**
         * Applies the attributes from the given IRTypeRepresentation to the
         * given function.
         * If the function returns by sret-parameter, the attributes are applied
         * to the parameter, otherwise they are applied to the return-value.
         */
        void applyAttrToReturn(const IRTypeRepresentation &retRepr,
                               llvm::Function *func);

        /**
         * Applies the attributes from the given IRTypeRepresentation to the
         * given function.
         * The given argIdx specifies to which arguments of the function the
         * attributes are applied.
         */
        void applyAttrToParamRepr(const IRTypeRepresentation &pRepr,
                                  llvm::Function *func, unsigned int argIdx);

        /**
         * Applies the attributes from the given IRTypeRepresentation to the
         * given function-call.
         * If the function returns by sret-parameter, the attributes are applied
         * to the parameter, otherwise they are applied to the return-value.
         */
        void applyAttrToReturn(const IRTypeRepresentation &retRepr,
                               llvm::CallInst *call);

        /**
         * Applies the attributes from the given IRTypeRepresentation to the
         * given function-call.
         * The given argIdx specifies to which arguments of the call the
         * attributes are applied.
         */
        void applyAttrToParamRepr(const IRTypeRepresentation &pRepr,
                                  llvm::CallInst *call, unsigned int argIdx);

        /**
         * Helper that adds the sret attribute to the argument with the given
         * index of the given function.
         */
        void addSret(
            llvm::CallInst *call, unsigned int argNo, llvm::Type *rType);

        /**
         * Helper that adds the byval attribute to the argument with the given
         * index of the given function.
         */
        void addByVal(
            llvm::CallInst *call, unsigned int argNo, llvm::Type *type);

    private:
        /**
         * Pointers to the unified specification functions within irModule.
         * Raw pointers, ownership lies with module.
         */
        llvm::Function *specImply = nullptr;

        llvm::Function *specScAnd = nullptr;

        llvm::Function *specScOr = nullptr;

        llvm::Function *specSepConj = nullptr;

        // TODO: Implement these as maps instead
        llvm::SmallVector<std::tuple<
            IRTypeRepresentation, IRTypeRepresentation, llvm::Function *>>
            specOld;

        llvm::SmallVector<std::pair<IRTypeRepresentation, llvm::Function *>> specResult;

        llvm::SmallVector<std::pair<IRTypeRepresentation, llvm::Function *>> specBoundVar;

        llvm::Function *specExists = nullptr;

        llvm::Function *specForall = nullptr;

        llvm::Function *specForallSep = nullptr;

        llvm::Function *specFracOf = nullptr;

        llvm::Function *specPerm = nullptr;

        llvm::Function *specPtrBlockLength = nullptr;

        llvm::Function *specPtrBlockOffset = nullptr;

        llvm::Function *specPtrLength = nullptr;

        llvm::SmallVector<std::tuple<
            IRTypeRepresentation, IRTypeRepresentation, llvm::Function *>>
            specUnfolding;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqNew;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqSize;

        llvm::SmallVector<std::tuple<
            llvm::Type *, IRTypeRepresentation, llvm::Function *>>
            specSeqGet;

        // llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqPrepend;
        llvm::SmallVector<std::tuple<
            llvm::Type *, IRTypeRepresentation, llvm::Function *>>
            specSeqPrepend;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqEquals;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqSlice;

        llvm::SmallVector<std::tuple<
            llvm::Type *, IRTypeRepresentation, llvm::Function *>> specSeqSet;

        llvm::DenseMap<llvm::Type *, llvm::StructType *> specSeqTypes;

        /**
         * Replace all uses of the given source-language
         * specification function with a unified specification function.
         */
        void replaceCallsToSpecFunction(llvm::Function &srcSpecFunction);

        /**
         * Determines the type of the given call to a specification-function from
         * the source library and calls the appropriate transform-method of the
         * source-specific subclass.
         */
        llvm::CallInst *transformSpecCall(llvm::CallInst &oldCall);

        /**
         * Replaces the type of all alloca-instructions that allocate memory for
         * a type from the source-specific specification library.
         */
        void replaceSpecTypes();

        /**
         * Private helper-Functions
         */

        /**
         * Helper to generate the functions of the unified specification library
         */
        llvm::Function *createSpecLibFunction(
            const std::string &name,
            llvm::Type *returnType,
            llvm::SmallVector<llvm::Type *> argTypes);
        llvm::Function *createSpecLibFunction(
            const std::string &name,
            llvm::Type *returnType,
            llvm::SmallVector<llvm::Type *> argTypes,
            const std::string &labelName);

        /**
         * Attaches a metadata-node to the given declaration that identifies it
         * as a delaration from the Pallas specification.
         * The added node uses IrSpecConstants::PALLAS_SPEC_LIB_MARKER as an
         * identifer and contains a MDString with the given identifier.
         */
        void markAsSpecLibFunction(llvm::Function *specLib,
                                   const std::string &identifier);

        /**
         * Returns pointer to the boolean-type (i.e. Int1Ty)
         * in the current context.
         */
        inline llvm::Type *getBoolTy();

        /**
         * Returns pointer to the void-type in the current context.
         */
        inline llvm::Type *getVoidTy();
    };

}

#endif // PALLAS_SPECUNIFIER_H