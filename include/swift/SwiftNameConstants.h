#ifndef PALLAS_SWIFTNAMECONSTANTS_H
#define PALLAS_SWIFTNAMECONSTANTS_H

#include <string>

namespace sourcespec2ir
{
    /**
     * Various constants that are used to identify the swift-specification-functions. 
     */

    // Prefix of spec-function in the specification-library
    const std::string SWIFT_SPEC_MODULE_PREFIX = "$s10PallasSpec";

    // Prefix of generated wrapper-functions
    const std::string SWIFT_WRAPPER_PREFIX = "PALLAS_SPEC_";

    // Name of the swift placeholder-type for fractions
    const std::string SWIFT_SPEC_FRAC_TYPE = "T10PallasSpec4FracV";

    // Prefix of the specification-function for bound variables. 
    const std::string SWIFT_SPEC_BOUND_VAR_PREFIX = "_bv";




}

#endif // PALLAS_SWIFTNAMECONSTANTS_H