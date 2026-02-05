#include "SourceSyntaxResolver.h"

#include "PallasException.h"

#include <sstream>

namespace sourcespec2ir
{

    SourceSyntaxResolver::SourceSyntaxResolver()
    {
    }

    SrcTypeRepresentation SourceSyntaxResolver::getTypeForGhostArgDef(
        llvm::StringRef argDef)
    {
        // Find the last ' ' and return the string left of it.
        llvm::SmallVector<llvm::StringRef> tokens;
        auto splitPos = argDef.find_last_of(' ');
        if (splitPos == llvm::StringRef::npos)
        {
            std::stringstream stream;
            stream << "Failed to extract type from ghost argument definition: "
                   << argDef.str();
            throw utils::PallasException(stream.str());
        }
        return argDef.substr(0, splitPos).trim();
    }
}