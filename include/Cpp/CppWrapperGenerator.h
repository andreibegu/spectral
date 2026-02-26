#ifndef PALLAS_CPPWRAPPERGENERATOR_H
#define PALLAS_CPPWRAPPERGENERATOR_H

#include "SourceWrapperGenerator.h"

namespace sourcespec2ir
{

    /**
     * Wrapper-generator for C++.
     */
    class CppWrapperGenerator : public SourceWrapperGenerator
    {
    public:
        CppWrapperGenerator(
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

    protected:
        // Overwritten methods to customize the default-WrapperGenerator
        SrcTypeRepresentation getVoid() override;
        void addConst(SrcTypeRepresentation &baseType) override;
        void addPointer(SrcTypeRepresentation &baseType) override;
        void addReference(SrcTypeRepresentation &baseType) override;
        void addTypedef(SrcTypeRepresentation &baseType,
                        const std::string &typedefName) override;
        void addArrayDim(SrcTypeRepresentation &baseType,
                         const std::string &dimSize);
        std::string addNamespace(const std::string &parentNamespace,
                                 const std::string &childNamespace) override;
        void modifySrcWithWrappers(std::list<SrcLine> &srcLines) override;
    };

}

#endif // PALLAS_CPPWRAPPERGENERATOR_H