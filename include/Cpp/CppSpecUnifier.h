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
     * 
     * TODO: Try to de-duplicate the 1:1-transformations that are identical to 
     *       those in the CSpecUnifier. 
     */
    class CppSpecUnifier : public SpecUnifier
    {
    public:

        const std::string CPP_SPEC_NAMESPACE = "_ZN10pallasSpec";
        // Map names of functions of the specification-library to the 
        // corresponding enum-representation.
        const llvm::StringMap<SpecLibElement> EXACT_MATCH_NAME_MAP = {
            {"6_implyEbb", Imply},
            {"4_andEbb", ScAnd},
            {"3_orEbb", ScOr},
            {"7_existsEbb", Exists},
            {"7_forallEbb", Forall},
            {"8_forallSEbb", ForallSep},
            {"7_fracOfEii", FracOf}, 
            {"4_sepEbb", SepConj}
        };
        const llvm::StringMap<SpecLibElement> PREFIX_MATCH_NAME_MAP = {
            {"4_oldI", Old},
            {"7_resultI", Result},
            {"10_resultRefI", ResultRef},
            {"3_bvI", BoundVar},
            {"7_seqNewI", SeqNew},
            {"8_seqSizeI", SeqSize},
            {"7_seqGetI", SeqGet},
            {"11_seqPrependI", SeqPrepend},
            {"10_seqEqualsI", SeqEquals},
            {"9_seqSliceI", SeqSlice},
            {"7_seqSetI", SeqSet}, 
            {"5_PermI", Perm}, 
            {"15_ptrBlockLengthI", PtrBlockLength},
            {"15_ptrBlockOffsetI", PtrBlockOffset},
            {"10_ptrLengthI", PtrLength},
            {"10_unfoldingI", Unfolding}
        };

        const std::string CPP_SPEC_FRAC_TYPE = "struct.pallasSpec::frac";
        const std::string CPP_SPEC_SEQ_TYPE_BASE = "struct.pallasSpec::seq";

        CppSpecUnifier(const bool printDebugInfo);

    protected:
        std::optional<SpecLibElement> isSrcSpecificationFunction(
            const llvm::Function &f) override;

        bool isSrcSeqType(const llvm::StructType &t) override;

        llvm::Type *getContentTypeOfADTTy(const llvm::Type &t) override;

        llvm::Type *getSrcLibFracType() override;
    };

}

#endif // PALLAS_CPPSPECUNIFIER_H