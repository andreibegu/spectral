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

    SwiftWrapperGenerator::SwiftWrapperGenerator(
        const std::string srcInFile,
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations,
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers)
        : SourceWrapperGenerator(srcInFile, specDeclarations, wrapperMarkers)
    {
    }

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
            const auto varName = getArgName(utils::getSrcVarName(**it));
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
        // This is only required in simple examples, where the used type is not
        // used in the code itself. (e.g. Bool is used as a return type,
        // but the code does not contain any local variables of type Bool.).
        if (srcType == "$sytD")
            return "Void";
        if (srcType == "$sSbD")
            return "Bool";

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
        // TODO: We could also do the generation of the spec-lib functions using
        //       Swift-macros. However, that makes the compilation-process more
        //       complicated, as the macros have to loaded in.
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
                stream << "@_optimize(none) \n";
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
            stream << "@_optimize(none) \n";
            stream << "func _old(_ v: " << type << ") -> " << type
                   << " { fatalError() } \n";
            // Generate function for Perm
            stream << "@inline(never) \n";
            stream << "@_optimize(none) \n";
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
            stream << "@_optimize(none) \n";
            stream << "func _bv" << bvType << "(_ id: String) -> " << bvType
                   << " { fatalError() } \n";
        }

        // Determine which ADT-types have to be generated
        // (using the user-provided declarations)
        llvm::SmallSet<std::string, 4> seqTypes;
        for (auto decl : specDeclarations)
        {
            llvm::StringRef declStr = decl.content;
            if (declStr.starts_with(SWIFT_SPEC_SEQ_DEF_MACRO_START) &&
                declStr.ends_with(">"))
            {
                auto typeStr = declStr.drop_front(SWIFT_SPEC_SEQ_DEF_MACRO_START.size())
                                   .drop_back(SWIFT_SPEC_SEQ_DEF_MACRO_END.size())
                                   .str();
                seqTypes.insert(typeStr);
            }
        }

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
                   << "     let e: " << eTy << " \n"
                   << "     let i1: Int64 \n"
                   << "     let i2: Int64 \n"
                   << "     let i3: Int64 \n"
                   << "     let i4: Int64 \n"
                   << "     let i5: Int64 \n"

                   // Constructor for new sequence
                   << "    @inline(never) \n"
                   << "    @_optimize(none) \n"
                   << "    init() { fatalError() } \n"
                   // Access element of sequence
                   << "    @inline(never) \n"
                   << "    @_optimize(none) \n"
                   << "    subscript(index: Int64) -> " << eTy << " { fatalError() } \n"
                   // Size of sequence
                   << "    @inline(never) \n"
                   << "    @_optimize(none) \n"
                   << "    func size() -> Int64 { fatalError() } \n"
                   // Prepend element to sequence
                   << "    @inline(never) \n"
                   << "    @_optimize(none) \n"
                   << "    func prepend(_ e: " << eTy << ") -> " << seqTy << " { fatalError() } \n"
                   // Equality of sequences
                   << "    @inline(never) \n"
                   << "    @_optimize(none) \n"
                   << "    func equals(_ s2: " << seqTy << ") -> Bool { fatalError() } \n"

                   << "} \n";
        }

        stream << "// END - SpecLib-Functions \n";
        return stream.str();
    }

    std::string SwiftWrapperGenerator::getArgName(const std::string &origName)
    {
        if (origName == "self")
            return "_self";
        return origName;
    }

}