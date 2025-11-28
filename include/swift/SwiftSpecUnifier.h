#ifndef PALLAS_SWIFTSPECUNIFIER_H
#define PALLAS_SWIFTSPECUNIFIER_H

#include "SpecUnifier.h"
#include "SwiftNameConstants.h"

#include <llvm/ADT/StringMap.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace sourcespec2ir
{

    /**
     * Specialization of the SpecUnifier for Swift as a source-language.
     */
    class SwiftSpecUnifier : public SpecUnifier
    {
    public:
        // Map names of functions of the specification-library to the
        // corresponding enum-representation.

        const llvm::StringMap<SpecLibElement> EXACT_MATCH_NAME_MAP = {
            {"4FracVyACs5Int32V_AEtcfC", FracOf},
            {"7_forallyS2b_SbtF", Forall},
            {"10_forallSepyS2b_SbtF", ForallSep},
            {"7_existsyS2b_SbtF", Exists},
            {"10_unfoldingyS2b_SbtF", Unfolding}};

        // Map to identify specification-functions that are defined as
        // infix-operators in the specification-library.
        const llvm::StringMap<SpecLibElement> INFIX_OP_NAME_MAP = {
            {SWIFT_IMPLY_OP, Imply},
            {SWIFT_SCAND_OP, ScAnd},
            {SWIFT_SCOR_OP, ScOr},
            {SWIFT_SEP_OP, SepConj}};

        // Map to identify the spec-lib functions that are generated inside
        // of the wrapper-function based on their prefix.
        const llvm::StringMap<SpecLibElement> GENERATED_PREFIX_MAP = {
            {SWIFT_SPEC_BOUND_VAR_PREFIX, BoundVar},
            {SWIFT_SPEC_UNFOLDING_PREFIX, Unfolding}};

        // Map to identify the spec-lib functions that are generated inside
        // of the wrapper-function.
        // Because we use the DISubprogram, we can use the unmangled names.
        const llvm::StringMap<SpecLibElement> GENERATED_NAME_MAP = {
            {"_old", Old},
            {"_result", Result},
            {"_perm", Perm}};

        // Map to identify the spec-lib functions that are generated
        // inside of a struct that represents an ADT
        const llvm::StringMap<SpecLibElement> ADT_GENERATED_NAME_MAP = {
            {SWIFT_SPEC_SEQ_NEW, SeqNew},
            {SWIFT_SPEC_SEQ_SIZE, SeqSize},
            {SWIFT_SPEC_SEQ_GET, SeqGet},
            {SWIFT_SPEC_SEQ_PREPEND, SeqPrepend},
            {SWIFT_SPEC_SEQ_EQUALS, SeqEquals}};

        SwiftSpecUnifier(const bool printDebugInfo);

    protected:
        std::optional<SpecLibElement> isSrcSpecificationFunction(
            const llvm::Function &f) override;

        bool isSrcSeqType(const llvm::StructType &t) override;

        llvm::Type *getContentTypeOfADTTy(const llvm::Type &t) override;

        llvm::Type *getSrcLibFracType() override;

        llvm::CallInst *transformBoundVar(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformSeqGet(llvm::CallInst &oldCall) override;
    };

}

#endif // PALLAS_SWIFTSPECUNIFIER_H