#ifndef PALLAS_SWIFTDEMANGLER_H
#define PALLAS_SWIFTDEMANGLER_H

#include <string.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>

namespace sourcespec2ir
{

    /**
     * Driver class that invokes swift-demangle to demangle sSwift names. 
     * Does some caching to avoid invoking swift-demangle more often than 
     * necessary. 
     */
    class SwiftDemangler 
    {
    public:
        SwiftDemangler();

        /**
         * Demangles the given swift name. 
         * NOTE: The passed string is used as input to swift-demangle which is 
         * executed as a shell command. Therefore, make sure that the inputs are
         * not malicious!!!
         */
        std::string demangle(llvm::StringRef name);

    private:
        llvm::StringMap<std::string> nameCache;
    };

}

#endif // PALLAS_SWIFTDEMANGLER_H