#ifndef PALLAS_SPEC2IRCONFIG_H
#define PALLAS_SPEC2IRCONFIG_H

#include <string>

namespace sourcespec2ir
{
    namespace utils
    {
        /**
         * Struct to bundle configuration-options that determine
         * how much information should be printed.
         */
        struct OutputConfig
        {
            const bool printMatchingInfo;
            const bool printUnificationInfo;
            const bool printDebugInfo;

            OutputConfig(const bool printMatchingInfo,
                         const bool printUnificationInfo,
                         const bool printDebugInfo)
                : printMatchingInfo(printMatchingInfo),
                  printUnificationInfo(printUnificationInfo),
                  printDebugInfo(printDebugInfo)
            {
            }
        };

    }
}

#endif // PALLAS_SPEC2IRCONFIG_H