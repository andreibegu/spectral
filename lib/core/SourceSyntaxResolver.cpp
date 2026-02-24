#include "SourceSyntaxResolver.h"
#include "PallasException.h"

#include <sstream>

#include <llvm/ADT/StringExtras.h>

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
        return SrcTypeRepresentation(argDef.substr(0, splitPos).trim());
    }

    std::string SourceSyntaxResolver::getArgName(llvm::StringRef argDef)
    {
        llvm::SmallVector<llvm::StringRef> tokens;
        argDef.split(tokens, ' ', -1, false);
        return tokens.back().str();
    }

    std::string SourceSyntaxResolver::getGhostMethodName(llvm::StringRef impl)
    {
        // Get index of first '('
        auto endIdx = impl.find('(');
        if (endIdx == llvm::StringRef::npos)
            return "";
        // Drop '(' and everything after
        auto prefix = impl.substr(0, endIdx).trim();
        // Split on whitepace
        llvm::SmallVector<llvm::StringRef> fragments;
        llvm::SplitString(prefix, fragments);
        if (fragments.empty())
            return "";
        // Return last fragment
        return fragments.back().str();
    }

    llvm::SmallVector<std::string> SourceSyntaxResolver::getGhostMethodArgs(
        llvm::StringRef impl)
    {
        // Get the string between the first '(' and ')'
        auto openIdx = impl.find('(');
        auto closeIdx = impl.find(')');
        if (openIdx == llvm::StringRef::npos ||
            closeIdx == llvm::StringRef::npos ||
            closeIdx <= openIdx)
        {
            throw utils::PallasException(
                "Invalid argument list of ghost method / function.");
        }
        auto argLen = closeIdx - (openIdx + 1);
        auto argStr = impl.substr(openIdx + 1, argLen);

        // Split arguments on ','
        llvm::SmallVector<llvm::StringRef> args;
        argStr.split(args, ',');

        // Trim whitespace and convert to string
        llvm::SmallVector<std::string> res;
        for (const auto &arg : args)
            res.push_back(arg.trim().str());

        return res;
    }

    std::string SourceSyntaxResolver::getGhostMethodRetTy(llvm::StringRef impl)
    {
        // We assume that everything in front of the funcion-name is the
        // return type. I.e. 'TYPE name (....'
        auto prefix = impl.take_while([](char c)
                                      { return c != '('; })
                          .trim();
        auto tStr = prefix.substr(0, prefix.find_last_of(" \t\n\v\f\r"));
        return tStr.trim().str();
    }


}