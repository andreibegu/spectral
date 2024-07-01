#ifndef PALLAS_CPPSPECUNIFIER_H
#define PALLAS_CPPSPECUNIFIER_H

#include "SpecUnifier.h"

#include <llvm/ADT/StringMap.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace sourcespec2ir
{

    /**
     * Specialization of the SpecUnifier for C as a source-language. 
     */
    class CppSpecUnifier : public SpecUnifier
    {
    public:
        /*
        // Map names of functions of the specification-library to the corresponding enum-representation.
        const llvm::StringMap<SpecLibElement> EXACT_MATCH_NAME_MAP = {
            {"imply", Imply},
            {"exists", Exists},
            {"forall", Forall},
            {"forallP", ForallSep},
            {"fracOf", FracOf},
            {"Perm", Perm},
        };
        const llvm::StringMap<SpecLibElement> PREFIX_MATCH_NAME_MAP = {
            {"PALLAS_OLD___", Old},
            {"PALLAS_RESULT___", Result},
            {"PALLAS_BOUND_VAR___", BoundVar},
            {"PALLAS_SEQ_NEW___", SeqNew},
            {"PALLAS_SEQ_SIZE___", SeqSize},
            {"PALLAS_SEQ_AT___", SeqGet},
            {"PALLAS_SEQ_APPEND___", SeqAppend},
        };

        const std::string C_SPEC_FRAC_TYPE = "struct.frac";
        const std::string C_SPEC_SEQ_TYPE_BASE = "PALLAS_SEQ___";
        */

        CppSpecUnifier(const bool printDebugInfo);

    protected:

        std::optional<SpecLibElement> isSrcSpecificationFunction(
            const llvm::Function &f) override;

        bool isSrcSeqType(const llvm::StructType &t) override;

        llvm::Type *getContentTypeOfADTTy(const llvm::Type &t) override;

        llvm::Type *getSrcLibFracType() override;

        llvm::CallInst *transformImply(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformOld(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformResult(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformBoundVar(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformExists(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformForall(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformForallSep(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformFracOf(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformPerm(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformSeqNew(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformSeqSize(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformSeqGet(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformSeqAppend(llvm::CallInst &oldCall) override;
    };

}

#endif // PALLAS_CPPSPECUNIFIER_H