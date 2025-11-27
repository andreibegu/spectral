#include "wrapper/SrcTypeRepresentation.h"

namespace sourcespec2ir
{
    SrcTypeRepresentation::SrcTypeRepresentation(llvm::StringRef prefix,
                                                 llvm::StringRef suffix)
        : prefix(prefix), suffix(suffix)
    {
    }
}