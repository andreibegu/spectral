#ifndef PALLAS_IRSPECCONSTANTS_H
#define PALLAS_IRSPECCONSTANTS_H

#include <string>

namespace sourcespec2ir
{

    /* 
     * String-constants that are used as identifiers for the metadata-nodes that are
     * used to define the specifications on the IR level. 
     */
    const std::string PALLAS_FUN_CONTRACT_BLOCK = "pallas.fcontract";

    const std::string PALLAS_ENSURES = "pallas.ensures";

    const std::string PALLAS_REQUIRES = "pallas.requires";


}

#endif // PALLAS_IRSPECCONSTANTS_H