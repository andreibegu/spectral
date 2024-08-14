#ifndef PALLAS_SPECUNIFIER_H
#define PALLAS_SPECUNIFIER_H

#include <optional>
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
            Imply,     // Implication
            Old,       // Old value of variable
            Result,    // Result in fucntion contract
            BoundVar,  // Bound variable of a quantifier
            Exists,    // Existential quantifier
            Forall,    // Universal quantifier
            ForallSep, // Universal quantifier with separating conjunction
            FracOf,    // Construct fraction from two ints
            Perm,      // Assert permission over memory location
            SeqNew,    // New sequence
            SeqSize,   // Size of sequence
            SeqGet,    // Get element from sequence
            SeqAppend  // Append element to sequence
        };

        SpecUnifier(const bool printDebugInfo);

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
         * These functions should be implemented by the source-language specific
         * specialization of the SpecUnifier.
         */
        virtual llvm::CallInst *transformImply(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformOld(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformResult(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformBoundVar(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformExists(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformForall(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformForallSep(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformFracOf(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformPerm(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformSeqNew(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformSeqSize(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformSeqGet(llvm::CallInst &oldCall) = 0;

        virtual llvm::CallInst *transformSeqAppend(llvm::CallInst &oldCall) = 0;

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
         * Returns pointer to the unified specification-function for the
         * \old-construct of the given type.
         */
        llvm::Function *getUSpecOld(llvm::Type *type);

        /**
         * Returns pointer to the unified specification-function for the
         * \result-construct of the given type.
         */
        llvm::Function *getUSpecResult(llvm::Type *type);

        /**
         * Returns pointer to the specification-function for the bound-variables
         * construct of the given type.
         */
        llvm::Function *getUSpecBoundVar(llvm::Type *type);

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
         */
        llvm::Function *getUSpecSeqGet(llvm::Type *type);

        /**
         * Returns a pointer to the unified specification-function for
         * appending an element to a sequence with the given content type.
         */
        llvm::Function *getUSpecSeqAppend(llvm::Type *type);

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
         * Helper that adds the sret attribute to the argument with the given 
         * index of the given function. 
         */
        void addSret(
            llvm::CallInst* call, unsigned int argNo, llvm::Type *rType);

    private:
        /**
         * Pointers to the unified specification functions within irModule.
         * Raw pointers, ownership lies with module.
         */
        llvm::Function *specImply = nullptr;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specOld;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specResult;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specBoundVar;

        llvm::Function *specExists = nullptr;

        llvm::Function *specForall = nullptr;

        llvm::Function *specForallSep = nullptr;

        llvm::Function *specFracOf = nullptr;

        llvm::Function *specPerm = nullptr;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqNew;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqSize;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqGet;

        llvm::DenseMap<llvm::Type *, llvm::Function *> specSeqAppend;

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