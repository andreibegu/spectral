#ifndef PALLAS_SPECUNIFIER_H
#define PALLAS_SPECUNIFIER_H

#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

namespace sourcespec2ir
{

    /**
     * Class that converts the IR-level specifications from the surce-language sepcific format to the unified
     * specification format of Pallas.
     * This involves replacing the calls to source-language specific specification-functions to calls with
     * the unified pallas-specification-functions.
     *
     * Does not take ownership of the passed Module.
     *
     * TODO: separate the source-language specific aspects into virtual functions and implement in subclass.
     * - Add an enum with a case for all possible specification-constructs (i.e. imply, exists, ...)
     * - Make "isSrcSpecificationFunction" return an optinal of this enum
     * - Add virtual methods for every specification-construct to replace the more general "createSpecCall" method.
     */
    class SpecUnifier
    {
    public:
        const std::string C_SPEC_IMPLY = "imply";
        const std::string C_SPEC_OLD_BASE = "PALLAS_OLD___";
        const std::string C_SPEC_RESULT_BASE = "PALLAS_RESULT___";
        const std::string C_SPEC_BOUND_VAR = "PALLAS_BOUND_VAR___";
        const std::string C_SPEC_EXISTS = "exists";
        const std::string C_SPEC_FORALL = "forall";
        const std::string C_SPEC_FORALL_SEP = "forallP";


        void unifySpecifications(llvm::Module *irModule, llvm::LLVMContext *irContext);

    protected:
        /**
         * Return wether or not the given function is a source-language specific specification function.
         */
        bool isSrcSpecificationFunction(const llvm::Function &f);

        /* Return pointer to the specification-function for implications.
         * If the declaration does not exist yet, it is created.
         */
        llvm::Function *getImplySpec();

        /* Returns a pointer to the specification-function for the \old-construct.
         * If the declaration does not exist yet, it is created. For every returned type,
         * a different function is created.
         */
        llvm::Function *getOldSpec(llvm::Type *type);

        /* Returns a pointer to the specification-function for the \result-construct.
         * If the declaration does not exist yet, it is created. For every returned type,
         * a different function is created.
         */
        llvm::Function *getResultSpec(llvm::Type *type);

        /* Returns a pointer to the specification-function for the bound-variables-construct.
         * If the declaration does not exist yet, it is created. For every returned type,
         * a different function is created.
         */
        llvm::Function *getBoundVarSpec(llvm::Type *type);

        /* Return pointer to the specification-function for the existential quantifier. 
         * If the declaration does not exist yet, it is created.
         */
        llvm::Function *getExistsSpec();

        /* Return pointer to the specification-function for the universal quantifier. 
         * If the declaration does not exist yet, it is created.
         */
        llvm::Function *getForallSpec();

        /* Return pointer to the specification-function for the separating universal quantifier. 
         * If the declaration does not exist yet, it is created.
         */
        llvm::Function *getForallSepSpec();

        /**
         * Creates a new function-call that is used to replace the call-instruction that was passed
         * as a parameter.
         * Ownership of the newly created instruction is transfered to the caller.
         */
        llvm::CallInst *createSpecCall(llvm::CallInst &oldCall);

    private:
        // Pointers to the loaded llvm-ir module and context
        llvm::Module *irModule;
        llvm::LLVMContext *irContext;

        /**
         * Pointers to the unified specification functions within irModule.
         * Raw pointer, ownership lies with module.
         */
        llvm::Function *specImply = nullptr;

        int specOldCounter = 0; // Counter used to generate unique names for the different old-functions of the spec-lib.
        llvm::DenseMap<llvm::Type *, llvm::Function *> specOld;

        int specResultCounter = 0; // Counter used to generate unique names for the different result-functions of the spec-lib.
        llvm::DenseMap<llvm::Type *, llvm::Function *> specResult;

        int specBoundVarCounter = 0; // Counter used to generate unique names for the different bound-variable-functions of the spec-lib.
        llvm::DenseMap<llvm::Type *, llvm::Function *> specBoundVar;

        llvm::Function *specExists = nullptr;

        llvm::Function *specForall = nullptr;

        llvm::Function *specForallSep = nullptr;

        /**
         * Replace all uses of the given source-language specific specification function with a
         * unified specification function.
         */
        void replaceCallsToSpecFunction(llvm::Function &srcSpecFunction);

        /**
         * Helper to generate the functions of the unified specification library
         */
        llvm::Function* createSpecLibFunction(const std::string &name, llvm::Type* returnType, llvm::SmallVector<llvm::Type*> argTypes);

        /**
         * Attaches a metadata-node to the given declaration that identifies it as a delaration
         * from the Pallas specification.
         * The added node uses IrSpecConstants::PALLAS_SPEC_LIB_MARKER as an identifer and
         * contains a MDString with the gien identifier.
         */
        void markAsSpecLibFunction(llvm::Function *specLib, const std::string &identifier);
    };

}

#endif // PALLAS_SPECUNIFIER_H