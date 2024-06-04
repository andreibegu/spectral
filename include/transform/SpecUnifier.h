#ifndef PALLAS_SPECUNIFIER_H
#define PALLAS_SPECUNIFIER_H

#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

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

        /**
         * Creates a new function-call that is used to replace the call-instruction that was passed
         * as a parameter.
         * Ownership of the newly created instruction is transfered to the caller.
         */
        llvm::CallInst *createSpecCall(llvm::CallInst &oldCall);

        /**
         * Attaches a metadata-node to the given declaration that identifies it as a delaration
         * from the Pallas specification.
         * The added node uses IrSpecConstants::PALLAS_SPEC_LIB_MARKER as an identifer and
         * contains a MDString with the gien identifier.
         */
        void markAsSpecLibFunction(llvm::Function *specLib, const std::string &identifier);

    private:
        // Pointers to the loaded llvm-ir module and context
        llvm::Module *irModule;
        llvm::LLVMContext *irContext;

        /**
         * Pointer to the unified specification functions within irModule.
         * Raw pointer, ownership lies with module.
         */
        llvm::Function *specImply = nullptr;

        /**
         * Replace all uses of the given source-language specific specification function with a
         * unified specification function.
         */
        void replaceCallsToSpecFunction(llvm::Function &srcSpecFunction);
    };

}

#endif // PALLAS_SPECUNIFIER_H