#include "SwiftSpecUnifier.h"
#include "IrLookup.h"
#include "PallasException.h"
#include "SwiftNameConstants.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/Support/raw_ostream.h>

namespace sourcespec2ir
{

    SwiftSpecUnifier::SwiftSpecUnifier(const bool printDebugInfo)
        : SpecUnifier(printDebugInfo)
    {
    }

    std::optional<SpecUnifier::SpecLibElement>
    SwiftSpecUnifier::isSrcSpecificationFunction(const llvm::Function &f)
    {
        auto fName = f.getName().str();

        // Check if the spec-function is on of the functions from the
        // spec-library-module.
        if (llvm::StringRef(fName).startswith(SWIFT_SPEC_MODULE_PREFIX))
        {
            // The name starts with the prefix of the spec-module
            auto nameWithoutPrefix = fName.substr(SWIFT_SPEC_MODULE_PREFIX.size());
            // Check name-matches
            auto match = EXACT_MATCH_NAME_MAP.find(nameWithoutPrefix);
            if (match != EXACT_MATCH_NAME_MAP.end())
                return std::make_optional<SpecLibElement>(match->getValue());
        }

        // Check for static functions from the spec-module, which have a different prefix
        if (fName == SWIFT_IMPLY_OP)
            return std::make_optional(Imply); 
        if (fName == SWIFT_SEP_OP)
            return std::make_optional(SepConj);

        auto sProg = f.getSubprogram();
        if (sProg == nullptr || sProg->getScope() == nullptr)
            return std::nullopt;
        auto scope = sProg->getScope();

        // Check if the function is a spec-function directly within the wrapper-function
        if (scope->getName().startswith(SWIFT_WRAPPER_PREFIX))
        {
            // Check for exact name matches
            const auto fName = sProg->getName();
            auto match = GENERATED_NAME_MAP.find(fName);
            if (match != GENERATED_NAME_MAP.end())
                return std::make_optional<SpecLibElement>(match->getValue());

            // Check prefix-matches
            for (auto key : GENERATED_PREFIX_MAP.keys())
            {
                if (llvm::StringRef(fName).startswith(key))
                {
                    auto value = GENERATED_PREFIX_MAP.lookup(key);
                    return std::make_optional<SpecLibElement>(value);
                }
            }
        }

        // Check if the function is a generated spec-function inside of a struct
        // that represents an ADT
        if (auto scopeStruct = llvm::dyn_cast<llvm::DICompositeType>(scope))
        {
            if (!(scopeStruct->getTag() == llvm::dwarf::Tag::DW_TAG_structure_type))
                return std::nullopt;
            if (!scopeStruct->getName().starts_with(SWIFT_SPEC_SEQ_TY_PREFIX))
                return std::nullopt;
            if (scopeStruct->getScope() == nullptr ||
                !scopeStruct->getScope()->getName().startswith(SWIFT_WRAPPER_PREFIX))
                return std::nullopt;

            // Check for name matches
            auto match = ADT_GENERATED_NAME_MAP.find(sProg->getName());
            if (match != ADT_GENERATED_NAME_MAP.end())
                return std::make_optional<SpecLibElement>(match->getValue());
        }

        return std::nullopt;
    }

    llvm::Type *SwiftSpecUnifier::getContentTypeOfADTTy(const llvm::Type &t)
    {
        if (auto *adtType = llvm::dyn_cast<llvm::StructType>(&t))
        {
            auto cType = adtType->getElementType(0);
            if (llvm::isa<llvm::PointerType>(cType))
                throw utils::PallasException(
                    "ADTs that contain reference-types are not supported yet");
            // In Swift, types like float are sometimes wrapped in a
            // packed struct (<{ float }>) with only one value.
            if (auto cTyStruct = llvm::dyn_cast<llvm::StructType>(cType))
            {
                if (cTyStruct->getNumElements() == 1 && cTyStruct->isPacked())
                    return cType = cTyStruct->getElementType(0);
                return cType;
            }
            return cType;
        }
        throw utils::PallasException(
            "Failed to extract the content-type of the ADT. Expected a struct.");
    }

    bool SwiftSpecUnifier::isSrcSeqType(const llvm::StructType &t)
    {
        const auto tName = t.getName();
        const bool containsWrapperPrefix = tName.contains(SWIFT_WRAPPER_PREFIX);
        const bool containsSeqPrefix = tName.contains(SWIFT_SPEC_SEQ_TY_PREFIX);
        return containsWrapperPrefix && containsSeqPrefix;
    }

    llvm::Type *SwiftSpecUnifier::getSrcLibFracType()
    {
        return llvm::StructType::getTypeByName(*irContext, SWIFT_SPEC_FRAC_TYPE);
    }

    llvm::CallInst *SwiftSpecUnifier::transformBoundVar(llvm::CallInst &oldCall)
    {
        /*
        The call to the _bv-functions looks something like this in the IR
        generated by the swift-compiler:
        %1 = call swiftcc { i64, ptr } @"$sSS21_builtinStringLiteral17utf8CodeUnitCount7isASCIISSBp_BwBi1_tcfC"(ptr @.str.1.x, i64 1, i1 true)
        %2 = extractvalue { i64, ptr } %1, 0, !dbg !62
        %3 = extractvalue { i64, ptr } %1, 1, !dbg !62
        %4 = call swiftcc i64 @"$s6output3foo1iSbSi_tF6_bvIntL_ySiSSF"(i64 %2, ptr %3)

        From the call to the _bv-function, we extract the pointer to the
        string-literal (@.str.1.x) and pass this to the call of the unified
        spec function.
        */

        auto type = utils::getReturnType(*oldCall.getCalledFunction());
        const auto isStructT = llvm::isa<llvm::StructType>(type);

        //
        //  Extract the pointer to the string-literal
        //
        if ((!isStructT && oldCall.arg_size() != 2) ||
            (isStructT && oldCall.arg_size() != 3))
            throw utils::PallasException("Unexpected signature of bound variable");
        // Get pointer to string-literal:
        auto ptrToStr = isStructT ? oldCall.getArgOperand(2) : oldCall.getArgOperand(1);
        // Get extractvalue-instruction that decomposes result from string-constructor
        auto extractInst = llvm::dyn_cast<llvm::ExtractValueInst>(ptrToStr);
        if (!extractInst)
            throw utils::PallasException(
                "Failed to unify bound-variable (Expected extractvalue)");
        // Get the call to the string-constructor
        auto strConstrCall = llvm::dyn_cast<llvm::CallInst>(
            extractInst->getAggregateOperand());
        // TODO: Checking for the constructor-name might be to fragile?!
        if (!strConstrCall ||
            strConstrCall->getCalledFunction()->getName().str() != SWIFT_STRING_CONSTR_LITERAL ||
            strConstrCall->arg_size() != 3)
            throw utils::PallasException(
                "Failed to unify bound-variable (Expected String constructor)");
        // Get pointer to the string literal
        auto literalPtr = strConstrCall->getArgOperand(0);
        if (!literalPtr || !llvm::isa<llvm::GlobalVariable>(literalPtr))
            throw utils::PallasException(
                "Failed to unify bound-variable (Expected global variable)");

        // TODO: Remove call to BridgeRelease if it exists (and if we run dce)

        if (llvm::isa<llvm::StructType>(type))
        {
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(0), literalPtr};
            auto newCall = llvm::CallInst::Create(getUSpecBoundVar(type), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::getWithStructRetType(*irContext, type));
            return newCall;
        }
        else
        {
            return llvm::CallInst::Create(getUSpecBoundVar(type), {literalPtr});
        }
    }

    llvm::CallInst *SwiftSpecUnifier::transformSeqGet(llvm::CallInst &oldCall)
    {
        // Identical to the default-implementation, but the order of index and 
        // sequence are swapped. 

        // Check the signature of the old call to determine if a
        // struct-type is returned.
        bool containsStruct = oldCall.getCalledFunction()->getReturnType() ==
                              llvm::Type::getVoidTy(*irContext);
        if (containsStruct)
        {
            // Content-type is struct-type
            auto cType = getContentTypeOfADT(*oldCall.getArgOperand(2));
            llvm::SmallVector<llvm::Value *, 4> newArgs = {
                oldCall.getArgOperand(0),
                oldCall.getArgOperand(2),
                oldCall.getArgOperand(1)};
            auto newCall = llvm::CallInst::Create(getUSpecSeqGet(cType), newArgs);
            addSret(newCall, 0, cType);
            newCall->addParamAttr(1, llvm::Attribute::NoUndef);
            addByVal(newCall, 1, getUSpecSeqType(cType));
            newCall->addParamAttr(2, llvm::Attribute::NoUndef);
            return newCall;
        }
        else
        {
            // Content-type is a non-struct-type
            auto cType = getContentTypeOfADT(*oldCall.getArgOperand(1));
            llvm::SmallVector<llvm::Value *, 2> newArgs = {
                oldCall.getArgOperand(1),
                oldCall.getArgOperand(0)};
            auto newCall = llvm::CallInst::Create(getUSpecSeqGet(cType), newArgs);
            newCall->addParamAttr(0, llvm::Attribute::NoUndef);
            addByVal(newCall, 0, getUSpecSeqType(cType));
            newCall->addParamAttr(1, llvm::Attribute::NoUndef);
            return newCall;
        }
    }

} // namespace sourcespec2ir