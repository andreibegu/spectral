#ifndef PALLAS_SWIFTSYNTAXRESOLVER_H
#define PALLAS_SWIFTSYNTAXRESOLVER_H

#include "SourceSyntaxResolver.h"

namespace sourcespec2ir
{

    /**
     * Driver class that invokes swiftc to compile specifications to LLVM-IR.
     */
    class SwiftSyntaxResolver : public SourceSyntaxResolver
    {
    public:
        SwiftSyntaxResolver();

        SrcTypeRepresentation getTypeForGhostArgDef(
            llvm::StringRef argDef) override;
            
        std::string getArgName(llvm::StringRef argDef) override;

        std::string getGhostMethodRetTy(llvm::StringRef impl) override;
    };

}

#endif // PALLAS_SWIFTSYNTAXRESOLVER_H