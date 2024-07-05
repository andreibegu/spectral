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

        std::unique_ptr<SrcWrapperFunction> getWrappingFunction(
            llvm::ArrayRef<llvm::DILocalVariable *> srcVars, const std::string &expr) override;

        std::string assembleSrcCompilationUnit(
            const SrcWrapperFunction &wrapperFunction, 
            const llvm::ArrayRef<std::string> &declarations) override;

    protected:
        std::string addConst(const std::string &baseType) override;
        std::string addPointer(const std::string &baseType) override;
        std::string addNamespace(const std::string &parentNamespace,
            const std::string & childNamespace) override;

    private:
        /* Counter for generating names for the specification functions.
         * TODO: Replace with something more elaborate that takes exisiting names into account.
         */
        int specFunCnt = 0;
    };

}

#endif // PALLAS_CPPWRAPPERGENERATOR_H