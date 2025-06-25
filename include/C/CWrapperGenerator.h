#ifndef PALLAS_CWRAPPERGENERATOR_H
#define PALLAS_CWRAPPERGENERATOR_H

#include "SourceWrapperGenerator.h"

namespace sourcespec2ir
{

    /**
     * Wrapper-generator for C.
     */
    class CWrapperGenerator : public SourceWrapperGenerator
    {
    public:
        CWrapperGenerator(
            const std::string srcInFile,
            const llvm::ArrayRef<sourcespec2ir::SpecDeclaration> specDeclarations,
            const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers);

        std::string getWrapperFunctionString(
            const WrapperSignature &sig,
            const std::string &expr,
            const std::string &functionName) override;

        std::pair<size_t, size_t> getOffsetToExprStart(
            const SrcWrapperFunction &wFun) override;

    protected:
        std::string addConst(const std::string &baseType) override;
        std::string addPointer(const std::string &baseType) override;
        std::string addTypedef(const std::string &baseType,
                               const std::string &typedefName) override;
        void modifySrcWithWrappers(std::vector<SrcLine> &srcLines) override;
    };

}

#endif // PALLAS_CWRAPPERGENERATOR_H