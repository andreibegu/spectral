#include "SwiftSpecUnifier.h"
#include "MetadataUtils.h"
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
        auto opMatch = INFIX_OP_NAME_MAP.find(fName);
        if (opMatch != INFIX_OP_NAME_MAP.end())
            return std::make_optional<SpecLibElement>(opMatch->getValue());

        auto sProg = f.getSubprogram();
        if (sProg == nullptr || sProg->getScope() == nullptr)
            return std::nullopt;
        auto scope = sProg->getScope();

        // Check if the function is a spec-function directly within
        // a wrapper-function or a predicate definition.
        // if (auto *scopeFunc = llvm::dyn_cast<llvm::DISubprogram>(scope))
        if (llvm::isa<llvm::DISubprogram>(scope)) 
        {
            // TODO: Re-introduce this check and also check that the function could 
            //       be in a predicate definition.
           // bool inWrapper = scopeFunc->getName().startswith(SWIFT_WRAPPER_PREFIX);
            //if (inWrapper)
            //{
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
            //}
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

        auto retRepr = IRTypeRepresentation::getRetReprFromCall(oldCall);
        auto isPtrRet = retRepr->isReturnByParam();

        //
        //  Extract the pointer to the string-literal
        //
        if ((!isPtrRet && oldCall.arg_size() != 2) ||
            (isPtrRet && oldCall.arg_size() != 3))
            throw utils::PallasException("Unexpected signature of bound variable");
        // Get pointer to string-literal:
        auto ptrToStr = isPtrRet ? oldCall.getArgOperand(2) : oldCall.getArgOperand(1);
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

        // Build new arg-list
        llvm::SmallVector<llvm::Value *> newArgs;
        if (isPtrRet)
            newArgs.push_back(oldCall.getArgOperand(0));
        newArgs.push_back(literalPtr);

        auto newCall = llvm::CallInst::Create(getUSpecBoundVar(*retRepr), newArgs);
        applyAttrToReturn(*retRepr, newCall);
        return newCall;
    }

    llvm::CallInst *SwiftSpecUnifier::transformSeqGet(llvm::CallInst &oldCall)
    {
        // Identical to the default-implementation, but the order of index and
        // sequence are swapped.
        // Check if the result is returned as an sret-parameter
        auto isParamReturn = oldCall.getCalledFunction()->getReturnType()->isVoidTy();

        // Get the content-type of the sequence from the argument
        auto cType = getContentTypeOfADT(
            *oldCall.getArgOperand(isParamReturn ? 2 : 1));

        // Build type-representation of the return-type
        IRTypeRepresentation retTRepr;
        auto retAttrs = isParamReturn ? oldCall.getAttributes().getParamAttrs(0)
                                      : oldCall.getAttributes().getRetAttrs();
        retTRepr.addType(oldCall.getCalledFunction()->getReturnType(), retAttrs);

        // Build new arg-list by copying arguments and swapping position of
        // sequence and index.
        llvm::SmallVector<llvm::Value *> newArgs;
        if (isParamReturn)
        {
            newArgs.push_back(oldCall.getArgOperand(0));
            newArgs.push_back(oldCall.getArgOperand(2));
            newArgs.push_back(oldCall.getArgOperand(1));
        }
        else
        {
            newArgs.push_back(oldCall.getArgOperand(1));
            newArgs.push_back(oldCall.getArgOperand(0));
        }

        // Build the new call
        auto newCall = llvm::CallInst::Create(
            getUSpecSeqGet(cType, retTRepr), newArgs);

        // Add attributes:
        if (isParamReturn)
            for (auto a : retTRepr.getAttributes(0))
                newCall->addParamAttr(0, a);

        newCall->addParamAttr(isParamReturn ? 1 : 0, llvm::Attribute::NoUndef);
        addByVal(newCall, isParamReturn ? 1 : 0, getUSpecSeqType(cType));
        newCall->addParamAttr(isParamReturn ? 2 : 1, llvm::Attribute::NoUndef);

        if (!isParamReturn)
            for (auto a : retTRepr.getAttributes(0))
                newCall->addRetAttr(a);

        return newCall;
    }

} // namespace sourcespec2ir