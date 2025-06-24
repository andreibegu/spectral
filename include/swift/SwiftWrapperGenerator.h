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
            const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
            const std::string &expr,
            const std::string &functionName,
            const llvm::DIType *returnType) override;

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
        void modifySrcWithWrappers(std::vector<SrcLine> &srcLines) override;

    private:
        std::string generateSpecLibFunctions(
            const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
            const llvm::DIType *returnType,
            const std::string &expr);
    };

}

#endif // PALLAS_SWIFTWRAPPERGENERATOR_H