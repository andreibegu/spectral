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
            const llvm::ArrayRef<sourcespec2ir::WrapperLocMarker> wrapperMarkers);

        std::string getWrapperFunctionString(
            const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
            const std::string &expr,
            const std::string &functionName,
            const llvm::DIType *returnType) override;

        std::pair<size_t, size_t> getOffsetToExprStart(
            const SrcWrapperFunction &wFun) override;

    protected:
        // Overwritten methods to customize the default-WrapperGenerator
        std::string addConst(const std::string &baseType) override;
        std::string addPointer(const std::string &baseType) override;
        std::string addReference(const std::string &baseType) override;
        std::string addTypedef(const std::string &baseType,
                               const std::string &typedefName) override;
        std::string addNamespace(const std::string &parentNamespace,
                                 const std::string &childNamespace) override;
        void modifySrcWithWrappers(std::vector<SrcLine> &srcLines) override;

    private:
        // Used to resolve conflicts of var-names with reserved keywords.
        std::string getNameForArg(const std::string &origName);
    };

}

#endif // PALLAS_CPPWRAPPERGENERATOR_H