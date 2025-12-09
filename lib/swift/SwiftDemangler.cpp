#include "SwiftDemangler.h"

#include <array>
#include <cstdio>

#include "PallasException.h"

namespace sourcespec2ir
{

    SwiftDemangler::SwiftDemangler()
    {
    }

    std::string SwiftDemangler::demangle(llvm::StringRef name)
    {
        // Skip everything that cannot be a mangled Swift name
        if (!name.starts_with("$s"))
            return name.str(); 

        auto cacheElem = nameCache.find(name);
        if (cacheElem != nameCache.end())
            return cacheElem->getValue(); 

        std::string cmd = "swift-demangle --compact --simplified '" + name.str() + "'";

        std::array<char, 128> buf;

        auto *p = popen(cmd.c_str(), "r");
        if (p == nullptr)
            throw utils::PallasException(
                "Failed to demangle swift identifier. "
                "Make sure that 'swift-demangle' is available.");

        std::string demangledName;
        while (fgets(buf.data(), 128, p) != nullptr)
            demangledName += buf.data();
        auto err = pclose(p);

        if (err)
            throw utils::PallasException(
                "Failed to read demangled swift identifier.");

        if (demangledName.ends_with('\n'))
            demangledName.erase(demangledName.length()-1);

        nameCache.insert({name, demangledName});
        return demangledName;
    }

}