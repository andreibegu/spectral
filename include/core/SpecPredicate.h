#ifndef PALLAS_SPECPREDICATE_H
#define PALLAS_SPECPREDICATE_H

#include <string>

#include "wrapper/SrcLocation.h"

namespace sourcespec2ir
{

    /**
     * Represents a definition of a predicate in a source file.
     */
    struct SpecPredicate
    {
        const std::string signature;
        const std::string body;
        const SrcRange srcLoc;

        SpecPredicate(std::string signature, std::string body,
                      const SrcRange srcLoc)
            : signature(signature), body(body), srcLoc(srcLoc)
        {
        }
    };
}

#endif // PALLAS_SPECPREDICATE_H