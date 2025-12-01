#ifndef PALLAS_SPECDECLARATION_H
#define PALLAS_SPECDECLARATION_H

#include "wrapper/SrcLocation.h"

#include <string>

namespace sourcespec2ir
{

    /**
     * Struct that is used to represent an entry from the specifications
     * declaration-block.
     */
    struct SpecDeclaration
    {
        std::string content;
        SrcRange srcLoc;
        SpecDeclaration(std::string content, SrcRange srcLoc)
            : content(content), srcLoc(srcLoc)
        {
        }
    };
}

#endif // PALLAS_SPECDECLARATION_H