#ifndef PALLAS_CSPECUNIFIER_H
#define PALLAS_CSPECUNIFIER_H

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
    class CSpecUnifier : public SpecUnifier
    {
    public:
        // Map names of functions of the specification-library to the corresponding enum-representation.
        const llvm::StringMap<SpecLibElement> EXACT_MATCH_NAME_MAP = {
            {"_imply", Imply},
            {"_and", ScAnd},
            {"_or", ScOr},
            {"_exists", Exists},
            {"_forall", Forall},
            {"_forallS", ForallSep},
            {"_fracOf", FracOf},
            {"_Perm", Perm},
            {"_ptr_block_length", PtrBlockLength},
            {"_ptr_block_offset", PtrBlockOffset},
            {"_ptr_length", PtrLength},
            {"_sep", SepConj}
        };
        const llvm::StringMap<SpecLibElement> PREFIX_MATCH_NAME_MAP = {
            {"PALLAS_OLD___", Old},
            {"PALLAS_RESULT___", Result},
            {"PALLAS_BOUND_VAR___", BoundVar},
            {"PALLAS_UNFOLDING___", Unfolding},
            {"PALLAS_SEQ_NEW___", SeqNew},
            {"PALLAS_SEQ_SIZE___", SeqSize},
            {"PALLAS_SEQ_AT___", SeqGet},
            {"PALLAS_SEQ_PREPEND___", SeqPrepend},
            {"PALLAS_SEQ_EQ___", SeqEquals},
            {"PALLAS_SEQ_SLICE___", SeqSlice},
            {"PALLAS_SEQ_SET___", SeqSet}
        };

        const std::string C_SPEC_FRAC_TYPE = "struct.frac";
        const std::string C_SPEC_SEQ_TYPE_BASE = "PALLAS_SEQ___";

        CSpecUnifier(const bool printDebugInfo);

    protected:

        std::optional<SpecLibElement> isSrcSpecificationFunction(const llvm::Function &f) override;

        bool isSrcSeqType(const llvm::StructType &t) override;

        llvm::Type *getContentTypeOfADTTy(const llvm::Type &t) override;

        llvm::Type *getSrcLibFracType() override;

    };

}

#endif // PALLAS_CSPECUNIFIER_H