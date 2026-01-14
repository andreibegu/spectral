#ifndef PALLAS_GHOSTMETHOD_H
#define PALLAS_GHOSTMETHOD_H

#include <string>

#include "wrapper/SrcLocation.h"

namespace sourcespec2ir
{

    /**
     * Represents a definition of a ghost method or function in a source file.
     */
    struct GhostMethod
    {
        const bool isFunction;
        const std::string name;
        const std::string definition;

        // Location of the specification block of the ghost method
        // in the src-file
        const SrcRange srcLoc;

        // Location where the definition of the ghost-function
        // in the src-file starts (i.e. excl. the contract etc.).
        const SrcLocation defLoc;

        GhostMethod(bool isFunction, std::string name, std::string definition,
                    const SrcRange srcLoc, const SrcLocation defLoc)
            : isFunction(isFunction),
              name(name),
              definition(definition),
              srcLoc(srcLoc),
              defLoc(defLoc)
        {
        }
    };
}

#endif // PALLAS_GHOSTMETHOD_H