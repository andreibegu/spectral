#ifndef PALLAS_SRCWRAPPERFUNCTION_H
#define PALLAS_SRCWRAPPERFUNCTION_H

#include <string>

namespace sourcespec2ir
{

    /**
     * A struct that bundles the name and the string representation of a wrapper-function. 
     */
    struct SrcWrapperFunction {
        std::string name;
        std::string stringRepr;
        SrcWrapperFunction(std::string name, std::string stringRepr);
    };
}

#endif // PALLAS_SRCWRAPPERFUNCTION_H