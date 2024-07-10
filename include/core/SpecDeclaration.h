#ifndef PALLAS_SPECDECLARATION_H
#define PALLAS_SPECDECLARATION_H

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
        size_t srcFileLine;
        SpecDeclaration(std::string content, size_t srcFileLine)
            : content(content), srcFileLine(srcFileLine)
        {
        }
    };
}

#endif // PALLAS_SPECDECLARATION_H