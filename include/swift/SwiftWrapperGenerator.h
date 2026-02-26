#ifndef PALLAS_SWIFTWRAPPERGENERATOR_H
#define PALLAS_SWIFTWRAPPERGENERATOR_H

#include "SourceWrapperGenerator.h"
#include "SwiftDemangler.h"
#include <llvm/ADT/StringRef.h>
namespace sourcespec2ir
{

    /**
     * Wrapper-generator for Swift.
     */
    class SwiftWrapperGenerator : public SourceWrapperGenerator
    {
    public:
        SwiftWrapperGenerator(
            const std::string srcInFile,
            const llvm::ArrayRef<SpecDeclaration> specDeclarations,
            const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers,
            std::unique_ptr<SourceSyntaxResolver> syntaxResolver);

        std::string getWrapperFunctionString(
            const WrapperSignature &sig,
            const std::string &expr,
            const std::string &functionName) override;

        std::string getPredicateDefString(const std::string &name,
                                          const std::string &args,
                                          const std::string &body) override;

        std::pair<size_t, size_t> getOffsetToExprStart(
            const SrcWrapperFunction &wFun) override;

        std::pair<size_t, size_t> getOffsetToExprStart(
            const SpecPredicate &pDef) override;

        /*
         * In swift, the debug-info for e.g. the Boolean-type does not always
         * get resolved properly. Hence, manually patch this.
         */
        SrcTypeRepresentation getSrcReprOfType(const llvm::DIType *type) override;

    protected:
        // Overwritten methods to customize the default-WrapperGenerator
        void addConst(SrcTypeRepresentation &baseType) override;
        void addTypedef(SrcTypeRepresentation &baseType,
                        const std::string &typedefName) override;
        void addOtherQualifier(SrcTypeRepresentation &baseType,
                               const llvm::DIDerivedType &type) override;
        void modifySrcWithWrappers(std::list<SrcLine> &srcLines) override;

        std::string buildArgStr(const SrcTypeRepresentation &type,
                                const std::string &name) override;

    private:
        SwiftDemangler demangler;

        // TODO: Instead of doing this here, we might want to handle the
        //      predicate arguments similar to the arguments on external
        //      contracts and separate them during parsing!
        WrapperSignature getSigFromPredicateArgs(llvm::StringRef argStr);
        std::string generateSpecLibFunctions(
            const WrapperSignature &sig, const std::string &expr);
    };

}

#endif // PALLAS_SWIFTWRAPPERGENERATOR_H