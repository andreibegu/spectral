#ifndef PALLAS_SWIFTWRAPPERGENERATOR_H
#define PALLAS_SWIFTWRAPPERGENERATOR_H

#include "SourceWrapperGenerator.h"

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
            const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers);

        std::string getWrapperFunctionString(
            const WrapperSignature &sig,
            const std::string &expr,
            const std::string &functionName) override;

        std::pair<size_t, size_t> getOffsetToExprStart(
            const SrcWrapperFunction &wFun) override;

        /*
         * In swift, the debug-info for e.g. the Boolean-type does not always
         * get resolved properly. Hence, manually patch this.
         */
        std::string getSrcReprOfType(const llvm::DIType *type) override;

    protected:
        // Overwritten methods to customize the default-WrapperGenerator
        std::string addConst(const std::string &baseType) override;
        std::string addTypedef(const std::string &baseType,
                               const std::string &typedefName) override;
        void modifySrcWithWrappers(std::list<SrcLine> &srcLines) override;

        std::string buildArgStr(const std::string &type,
                                const std::string &name) override;

    private:
        std::string generateSpecLibFunctions(
            const WrapperSignature &sig, const std::string &expr);
    };

}

#endif // PALLAS_SWIFTWRAPPERGENERATOR_H