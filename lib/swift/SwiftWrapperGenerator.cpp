#include "SwiftWrapperGenerator.h"
#include "PallasException.h"
#include "SrcLookup.h"
#include "SwiftNameConstants.h"

#include <sstream>
#include <regex>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Type.h>

namespace sourcespec2ir
{
    std::string SwiftWrapperGenerator::getWrapperFunctionString(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const std::string &expr,
        const std::string &functionName,
        const llvm::DIType *returnType)
    {

        // Generate the required functions for the specification-library
        const auto specFuncs = generateSpecLibFunctions(srcVars, returnType,
                                                        expr);

        std::stringstream stream;
        stream << "func " << functionName << "(";

        // Construct arg-list
        for (auto it = srcVars.begin(), end = srcVars.end(); it != end; ++it)
        {
            if (it != srcVars.begin())
                stream << ", ";
            const auto varName = utils::getSrcVarName(**it);
            const auto varType = getSrcReprOfType((*it)->getType());
            stream << varName << ": " << varType;
        }
        stream << ") -> Bool ";

        // Construct body
        stream << "{ \n";
        stream << "return " << expr << "; \n";
        stream << "\n";
        stream << specFuncs;
        stream << "} \n";
        stream << "\n";

        return stream.str();
    }

    std::string SwiftWrapperGenerator::getSrcReprOfType(
        const llvm::DIType *type)
    {
        // Get src-representation of the type using default-implementation
        const auto srcType = SourceWrapperGenerator::getSrcReprOfType(type);

        // Manually handle some cases where the type is not correctly resolved.
        if (srcType == "$sytD")
            return "Void";

        return srcType;
    }

    std::pair<size_t, size_t> SwiftWrapperGenerator::getOffsetToExprStart(
        const SrcWrapperFunction &wFun)
    {
        return std::make_pair(1, 7);
    }

    std::string SwiftWrapperGenerator::addConst(const std::string &baseType)
    {
        // In swift, function-arguments are const by default
        return baseType;
    }

    std::string SwiftWrapperGenerator::addTypedef(
        const std::string &baseType, const std::string &typedefName)
    {
        return baseType;
    }

    void SwiftWrapperGenerator::modifySrcWithWrappers(std::vector<SrcLine> &srcLines)
    {
        // Add include for the specification-module to the top of the
        // generated file.
        srcLines.insert(srcLines.begin(),
                        SrcLine("import PallasSpec", 0));
    }

    /*
     * Helpers for spec-function generation
     */
    namespace
    {

        /**
         * Performs regex-matching in the given expression string to determine
         * the types for which spec-functions have to be generated.
         * Assumes that the function-names are built from a prefix followed
         * by the associated type (i.e. _bvInt("x") for a bound variable
         * of type Int).
         * The results are inserted into the set that is passed as a parameter.
         */
        void getTypesFromSuffix(const std::string &expr,
                                const std::string &prefix,
                                llvm::SmallSet<std::string, 4> &types)
        {
            std::regex re(prefix + "([^\\(]+)\\(");
            std::smatch matches;
            auto exprStr = expr;
            while (std::regex_search(exprStr, matches, re))
            {
                if (matches.size() < 2)
                    throw utils::PallasException(
                        "Failed extract type from function-name");
                types.insert(matches[1].str());
                exprStr = matches.suffix().str();
            }
        }

    }

    std::string SwiftWrapperGenerator::generateSpecLibFunctions(
        const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
        const llvm::DIType *returnType, const std::string &expr)
    {
        std::stringstream stream;
        stream << "// START - SpecLib-Functions \n";

        // Generate function for result
        if (returnType != nullptr)
        {
            // The spec-function is only a placeholder and not executed.
            // So we just throw a fatal error to get it to compile.
            const auto retTypeStr = getSrcReprOfType(returnType);
            if (retTypeStr != "Void")
            {
                stream << "@inline(never) \n";
                stream << "func _result() -> " << retTypeStr
                       << "{ fatalError() } \n";
            }
        }

        // Build a set of the argument-types to avoid duplicates
        llvm::SmallSet<std::string, 8> types;
        for (auto var : srcVars)
            types.insert(getSrcReprOfType(var->getType()));
        for (auto type : types)
        {
            // Generate function for old
            stream << "@inline(never) \n";
            stream << "func _old(_ v: " << type << ") -> " << type
                   << " { fatalError() } \n";
            // Generate function for Perm
            stream << "@inline(never) \n";
            stream << "func _perm(_ loc: " << type << ", _ f: Frac) -> Bool "
                   << "{ fatalError() } \n";
        }

        // Determine for which types the _bv-functions have to be generated.
        llvm::SmallSet<std::string, 4> bvTypes;
        getTypesFromSuffix(expr, SWIFT_SPEC_BOUND_VAR_PREFIX, bvTypes);
        // Generate spec-functions for bound variables:
        for (auto bvType : bvTypes)
        {
            stream << "@inline(never) \n";
            stream << "func _bv" << bvType << "(_ id: String) -> " << bvType
                   << " { fatalError() } \n";
        }

        // Determine which ADT-types have to be generated.
        llvm::SmallSet<std::string, 4> seqTypes;
        getTypesFromSuffix(expr, SWIFT_SPEC_SEQ_NEW_PREFIX, seqTypes);
        getTypesFromSuffix(expr, SWIFT_SPEC_SEQ_SIZE_PREFIX, seqTypes);
        getTypesFromSuffix(expr, SWIFT_SPEC_SEQ_GET_PREFIX, seqTypes);
        getTypesFromSuffix(expr, SWIFT_SPEC_SEQ_PREPEND_PREFIX, seqTypes);
        getTypesFromSuffix(expr, SWIFT_SPEC_SEQ_EQUALS_PREFIX, seqTypes);
        // Generate the sequence-types & the associated functions
        for (auto eTy : seqTypes)
        {
            // TODO: Generate a macro which in turn generates the spec-functions
            // this would be easier to read in the code and makes it possible to
            // immitate a generics-style syntax.
            // The declarations could be generated usinfg a body-macro and
            // the calls translated using a freestanding macro.
            const auto seqTy = SWIFT_SPEC_SEQ_TY_PREFIX + eTy;
            // First element encodes content-types, the other fields are filler
            // to prevent the stryct from being decomposed.
            stream << "struct " << seqTy << " { "
                   << "let e: " << eTy << " \n"
                   << "let i1: Int64 \n"
                   << "let i2: Int64 \n"
                   << "let i3: Int64 \n"
                   << "let i4: Int64 \n"
                   << "let i5: Int64 \n"
                   << "} \n";
            // New sequence
            stream << "@inline(never) "
                   << "func " << SWIFT_SPEC_SEQ_NEW_PREFIX << eTy << "() -> " << seqTy
                   << " { fatalError() } \n";
            // Size of sequence
            stream << "@inline(never) "
                   << "func " << SWIFT_SPEC_SEQ_SIZE_PREFIX << eTy 
                   << "(_ s: " << seqTy << ") -> Int64"
                   << " { fatalError() } \n";
            // Get on sequence
            stream << "@inline(never) "
                   << "func " << SWIFT_SPEC_SEQ_GET_PREFIX << eTy 
                   << "(_ s: " << seqTy << ", _ i: Int64) -> " << eTy
                   << " { fatalError() } \n";
            // Prepend to sequence
            stream << "@inline(never) "
                   << "func " << SWIFT_SPEC_SEQ_PREPEND_PREFIX << eTy << "(" 
                   << "_ e: " << eTy << ", " 
                   << "_ s: " << seqTy 
                   << ") -> " << seqTy
                   << " { fatalError() } \n";
            // Equality on sequence
            stream << "@inline(never) "
                   << "func " << SWIFT_SPEC_SEQ_EQUALS_PREFIX << eTy << "(" 
                   << "_ s1: " << seqTy << ", " 
                   << "_ s2: " << seqTy 
                   << ") -> Bool "
                   << " { fatalError() } \n";
            
        }

        stream << "// END - SpecLib-Functions \n";
        return stream.str();
    }

}