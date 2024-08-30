#ifndef PALLAS_SWIFTSPECUNIFIER_H
#define PALLAS_SWIFTSPECUNIFIER_H

#include "SpecUnifier.h"

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
            {"5implyyS2b_SbtF", Imply},
            {"3sepyS2b_SbtF", SepConj},
        };

        const llvm::StringMap<SpecLibElement> PREFIX_MATCH_NAME_MAP = {
            // {"old<", Old},
            // {"result<", Result},
        };

        const std::string SWIFT_SPEC_MODULE_PREFIX = "$s10PallasSpec";

        SwiftSpecUnifier(const bool printDebugInfo);

    protected:
        std::optional<SpecLibElement> isSrcSpecificationFunction(
            const llvm::Function &f) override;

        bool isSrcSeqType(const llvm::StructType &t) override;

        llvm::Type *getContentTypeOfADTTy(const llvm::Type &t) override;

        llvm::Type *getSrcLibFracType() override;

        llvm::CallInst *transformImply(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformSepConj(llvm::CallInst &oldCall) override;

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

        llvm::CallInst *transformSeqPrepend(llvm::CallInst &oldCall) override;

        llvm::CallInst *transformSeqEquals(llvm::CallInst &oldCall) override;
    };

}

#endif // PALLAS_SWIFTSPECUNIFIER_H