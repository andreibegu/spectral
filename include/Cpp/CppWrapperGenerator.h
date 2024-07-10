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
        std::string getWrapperFunctionString(
            const llvm::ArrayRef<llvm::DILocalVariable *> &srcVars,
            const std::string &expr,
            const std::string &functionName) override;

        std::string assembleSrcCompilationUnit(
            const SrcWrapperFunction &wrapperFunction,
            const llvm::ArrayRef<std::string> &declarations) override;

    protected:
        std::string addConst(const std::string &baseType) override;
        std::string addPointer(const std::string &baseType) override;
        std::string addNamespace(const std::string &parentNamespace,
                                 const std::string &childNamespace) override;
    };

}

#endif // PALLAS_CPPWRAPPERGENERATOR_H