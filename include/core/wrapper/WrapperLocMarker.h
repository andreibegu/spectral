#ifndef PALLAS_WRAPPERLOCMARKER_H
#define PALLAS_WRAPPERLOCMARKER_H

#include <optional>
#include <string>

#include "SrcLocation.h"

namespace sourcespec2ir
{

    /**
     * Represents a location-marker annotation.
     */
    struct WrapperLocMarker
    {
        const std::optional<std::string> scopeMatchingStr;
        const SrcRange markerLoc;

        WrapperLocMarker(const std::optional<std::string> scopeMatchingStr,
                         const SrcRange markerLoc)
            : scopeMatchingStr(scopeMatchingStr),
              markerLoc(markerLoc)
        {
        }
    };
}

#endif // PALLAS_WRAPPERLOCMARKER_H