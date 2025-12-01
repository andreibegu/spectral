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

        const std::string CPP_SPEC_NAMESPACE = "pallasSpec";
        // Map names of functions of the specification-library to the 
        // corresponding enum-representation.
        const llvm::StringMap<SpecLibElement> EXACT_MATCH_NAME_MAP = {
            {"_imply(bool, bool)", Imply},
            {"_and(bool, bool)", ScAnd},
            {"_or(bool, bool)", ScOr},
            {"_exists(bool, bool)", Exists},
            {"_forall(bool, bool)", Forall},
            {"_forallS(bool, bool)", ForallSep},
            {"_fracOf(int, int)", FracOf}, 
            {"_sep(bool, bool)", SepConj}
        };
        const llvm::StringMap<SpecLibElement> PREFIX_MATCH_NAME_MAP = {
            {"_old<", Old},
            {"_result<", Result},
            {"_bv<", BoundVar},
            {"_seqNew<", SeqNew},
            {"_seqSize<", SeqSize},
            {"_seqGet<", SeqGet},
            {"_seqPrepend<", SeqPrepend},
            {"_seqEquals<", SeqEquals}, 
            {"_Perm<", Perm}, 
            {"_ptrBlockLength<", PtrBlockLength},
            {"_ptrBlockOffset<", PtrBlockOffset},
            {"_ptrLength<", PtrLength},
            {"_unfolding<", Unfolding}
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