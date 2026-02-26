#include "SwiftSyntaxResolver.h"
#include "PallasException.h"

#include <sstream>

namespace sourcespec2ir
{

    SwiftSyntaxResolver::SwiftSyntaxResolver() {}

    SrcTypeRepresentation SwiftSyntaxResolver::getTypeForGhostArgDef(
        llvm::StringRef argDef)
    {
        // Find the ':' that separates the type
        auto [n, t] = argDef.trim().split(':');
        if (t.empty()) {
            std::stringstream s;
            s << "Failed to extract type from ghost argument definition: "
                   << argDef.str();
            throw utils::PallasException(s.str());
        } 

        return SrcTypeRepresentation(t.trim().str());
    }

    std::string SwiftSyntaxResolver::getArgName(llvm::StringRef argDef)
    {
        // Find the ':' that separates the type
        auto [n, t] = argDef.trim().split(':');
        return n.trim().str();
    }

    std::string SwiftSyntaxResolver::getGhostMethodRetTy(llvm::StringRef impl)
    {
        // Drop verything after the start of the function-body
        auto sig = impl.take_until([](char c)
                                   { return c == '{'; })
                       .trim();
        // Find '->'
        auto [l, r] = sig.split("->");
        if (!r.empty())
            return r.str();
        return "()";
    }

}