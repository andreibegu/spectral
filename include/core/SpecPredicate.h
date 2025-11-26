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
        const bool isInline;
        const std::string name;
        const std::string args;
        const std::string body;
        // Location where the predicate-definition starts in the src-file
        const SrcRange srcLoc;
        // Location where the body-expression in the src-file starts.
        const SrcLocation exprLoc;

        SpecPredicate(bool isInline, std::string name, std::string args,
                      std::string body, const SrcRange srcLoc,
                      const SrcLocation exprLoc)
            : isInline(isInline),
              name(name),
              args(args),
              body(body),
              srcLoc(srcLoc),
              exprLoc(exprLoc)
        {
        }
    };
}

#endif // PALLAS_SPECPREDICATE_H