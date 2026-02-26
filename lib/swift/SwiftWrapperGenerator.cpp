#include "SwiftWrapperGenerator.h"
#include "PallasException.h"
#include "SrcLookup.h"
#include "SwiftNameConstants.h"
#include "SwiftSyntaxResolver.h"

#include <sstream>
#include <regex>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/Type.h>

namespace sourcespec2ir
{

    SwiftWrapperGenerator::SwiftWrapperGenerator(
        const std::string srcInFile,
        const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations,
        const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers,
        std::unique_ptr<SourceSyntaxResolver> syntaxResolver)
        : SourceWrapperGenerator(
              srcInFile, specDeclarations, wrapperMarkers, 
              std::move(syntaxResolver)),
          demangler()
    {
    }

    std::string SwiftWrapperGenerator::getWrapperFunctionString(
        const WrapperSignature &sig,
        const std::string &expr,
        const std::string &functionName)
    {

        auto rType = sig.returnTy.has_value()
                         ? sig.returnTy.value().prefix.str().str()
                         : "Bool";
        // Generate the required functions for the specification-library
        const auto specFuncs = generateSpecLibFunctions(sig, expr);

        std::stringstream stream;
        stream << "func " << functionName << "(";

        // Construct arg-list
        bool firstArg = true;
        for (auto &arg : sig.args)
        {
            // Skip 'self'-arg as it gets added implicitly
            if (arg.ends_with(" self"))
                continue;
            if (!firstArg)
                stream << ", ";
            stream << arg;
            firstArg = false;
        }
        stream << ") -> " << rType << " ";

        // Construct body
        stream << "{ \n";
        stream << "return " << expr << "; \n";
        stream << "\n";
        stream << specFuncs;
        stream << "} \n";
        stream << "\n";

        return stream.str();
    }

    std::string SwiftWrapperGenerator::getPredicateDefString(
        const std::string &name, const std::string &args,
        const std::string &body)
    {
        auto sig = getSigFromPredicateArgs(args);
        const auto specFuncs = generateSpecLibFunctions(sig, body);

        std::stringstream s;
        s << "func " << name << "(" << args << ") -> Bool {\n"
          << "return " << body << "; \n"
          << "\n"
          << specFuncs
          << "}\n"
          << "\n";

        return s.str();
    }

    std::string SwiftWrapperGenerator::buildArgStr(
        const SrcTypeRepresentation &type, const std::string &name)
    {
        if (!type.suffix.empty())
            throw utils::PallasException(
                "Unexpected suffix in type representation");
        return name + ": " + type.prefix.str().str();
    }

    SrcTypeRepresentation SwiftWrapperGenerator::getSrcReprOfType(
        const llvm::DIType *type)
    {
        // Get src-representation of the type using default-implementation
        auto srcType = SourceWrapperGenerator::getSrcReprOfType(type);

        auto demangledPrefix = demangler.demangle(srcType.prefix);
        srcType.prefix = demangledPrefix;

        // Manually handle some cases where the type is not correctly resolved.
        // This is only required in simple examples, where the used type is not
        // used in the code itself. (e.g. Bool is used as a return type,
        // but the code does not contain any local variables of type Bool.).
        /*
        if (srcType.prefix == "$sytD")
            srcType.prefix = "Void";
        if (srcType.prefix == "$sSbD")
            srcType.prefix = "Bool";
        */
        return srcType;
    }

    std::pair<size_t, size_t> SwiftWrapperGenerator::getOffsetToExprStart(
        const SrcWrapperFunction &wFun)
    {
        return std::make_pair(1, 7);
    }

    std::pair<size_t, size_t> SwiftWrapperGenerator::getOffsetToExprStart(
        const SpecPredicate &pDef)
    {
        return std::make_pair(1, 7);
    }

    void SwiftWrapperGenerator::addConst(SrcTypeRepresentation &baseType)
    {
        // In swift, function-arguments are const by default
        return;
    }

    void SwiftWrapperGenerator::addTypedef(
        SrcTypeRepresentation &baseType, const std::string &typedefName)
    {
        return;
    }

    void SwiftWrapperGenerator::addOtherQualifier(
        SrcTypeRepresentation &baseType, const llvm::DIDerivedType &type)
    {
        if (type.getTag() == llvm::dwarf::DW_TAG_member)
            return;

        SourceWrapperGenerator::addOtherQualifier(baseType, type);
    }

    void SwiftWrapperGenerator::modifySrcWithWrappers(std::list<SrcLine> &srcLines)
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

        std::string getTypeFromArg(const std::string &arg)
        {
            // Split arg at the ':' into name and type
            auto splitIdx = arg.find(':');
            if (splitIdx == std::string::npos)
                throw utils::PallasException(
                    "Failed to get type from wrapper-argument: " + arg);
            auto tyStr = arg.substr(splitIdx + 1);
            // Strip leading spaces
            return tyStr.erase(0, tyStr.find_first_not_of(" "));
        }

    }

    std::string SwiftWrapperGenerator::generateSpecLibFunctions(
        const WrapperSignature &sig, const std::string &expr)
    {
        // TODO: We could also do the generation of the spec-lib functions using
        //       Swift-macros. However, that makes the compilation-process more
        //       complicated, as the macros have to loaded in.
        std::stringstream stream;
        stream << "// START - SpecLib-Functions \n";

        // Generate function for result
        if (sig.parentRetTy.has_value())
        {
            // The spec-function is only a placeholder and not executed.
            // So we just throw a fatal error to get it to compile.
            const auto &retTypeStr = sig.parentRetTy.value();
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
        for (auto &arg : sig.args)
            types.insert(getTypeFromArg(arg));
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

        // Generate spec-functions for _unfolding:
        llvm::SmallSet<std::string, 4> unfoldingTypes;
        getTypesFromSuffix(expr, SWIFT_SPEC_UNFOLDING_PREFIX, unfoldingTypes);
        for (auto t : unfoldingTypes)
        {
            // We generate a default-implementation for the Bool-type, so we can skip this here
            if (t == "")
                continue;
            stream << "@inline(never) \n"
                   << "@_optimize(none) \n"
                   << "func _unfolding" << t << "(_ pred: Bool, _ in: " << t << ") -> " << t << " "
                   << "{ fatalError() } \n";
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

    WrapperSignature SwiftWrapperGenerator::getSigFromPredicateArgs(llvm::StringRef argStr)
    {
        WrapperSignature sig;
        sig.parentRetTy = std::nullopt;

        // Split the argument list on ',' to get the types of the arguments
        llvm::SmallVector<llvm::StringRef> args;
        argStr.split(args, ',');
        for (auto arg : args)
            sig.addArg(arg.str());
        return sig;
    }
}