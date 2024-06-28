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

        std::unique_ptr<SrcWrapperFunction> getWrappingFunction(
            llvm::ArrayRef<llvm::DILocalVariable *> srcVars, const std::string &expr) override;

        std::string assembleSrcCompilationUnit(
            const SrcWrapperFunction &wrapperFunction, 
            const llvm::ArrayRef<std::string> &declarations) override;

        std::string getSrcReprOfType(const llvm::DIType *type) override;

    private:
        /* Counter for generating names for the specification functions.
         * TODO: Replace with something more elaborate that takes exisiting names into account.
         */
        int specFunCnt = 0;
    };

}

#endif // PALLAS_CWRAPPERGENERATOR_H