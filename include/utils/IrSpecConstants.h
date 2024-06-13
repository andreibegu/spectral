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

    // Metadata-identifier that is used to mark the IR specification functions
    const std::string PALLAS_SPEC_LIB_MARKER = "pallas.specLib";

    // Metadata-identifier that is used to mark generated functions that wrap specification expressions
    const std::string PALLAS_GENERATED_MARKER = "pallas.exprWrapper";

    /* Constants used to name specification-functions */
    const std::string PALLAS_SPEC_IMPLY = "pallas.imply";

    const std::string PALLAS_SPEC_OLD_BASE = "pallas.old";

    const std::string PALLAS_SPEC_RESULT_BASE = "pallas.result";

    const std::string PALLAS_SPEC_BOUND_VAR = "pallas.boundVar";

    const std::string PALLAS_SPEC_EXISTS = "pallas.exists";

    const std::string PALLAS_SPEC_FORALL = "pallas.forall";

    const std::string PALLAS_SPEC_FORALL_SEP = "pallas.forallSep";

    const std::string PALLAS_SPEC_FRAC_OF = "pallas.fracOf";

    const std::string PALLAS_SPEC_PERM = "pallas.perm";

    const std::string PALLAS_SPEC_SEQ_NEW_BASE = "pallas.seq.new";

    const std::string PALLAS_SPEC_SEQ_SIZE_BASE = "pallas.seq.size";

    const std::string PALLAS_SPEC_SEQ_GET_BASE = "pallas.seq.get";

    const std::string PALLAS_SPEC_SEQ_APPEND_BASE = "pallas.seq.append";


    /* Constants used to name specification-types */
    const std::string PALLAS_SPEC_TYPE_FRAC = "pallas.fracT";

    const std::string PALLAS_SPEC_TYPE_SEQ_BASE = "pallas.seq";


}

#endif // PALLAS_IRSPECCONSTANTS_H