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

    const std::string PALLAS_EXT_FUN_CONTRACT_BLOCK = "pallas.extContract";

    const std::string PALLAS_ENSURES = "pallas.ensures";

    const std::string PALLAS_REQUIRES = "pallas.requires";

    const std::string PALLAS_LOOP_INV_BLOCK = "pallas.loopInvBlock";

    const std::string PALLAS_LOOP_INV = "pallas.loopInv";

    const std::string PALLAS_STMNT_BLOCK = "pallas.stmntBlock";

    const std::string PALLAS_ASSERT = "pallas.assert";

    const std::string PALLAS_ASSUME = "pallas.assume";

    const std::string PALLAS_FOLD = "pallas.fold";

    const std::string PALLAS_UNFOLD = "pallas.unfold";

    const std::string PALLAS_GHOST_ASSIGN = "pallas.gAssign";

    const std::string PALLAS_GIVEN_BINDING_BLOCK = "pallas.givenBindings";

    const std::string PALLAS_GIVEN_BINDING = "pallas.givenBinding";

    const std::string PALLAS_YIELDS_BINDING_BLOCK = "pallas.yieldsBindings";

    const std::string PALLAS_YIELDS_BINDING = "pallas.yieldsBinding";

    // Metadata-identifier that is used to mark the IR specification functions
    const std::string PALLAS_SPEC_LIB_MARKER = "pallas.specLib";

    // Metadata-identifier that is used to mark generated functions that wrap specification expressions
    const std::string PALLAS_GENERATED_MARKER = "pallas.exprWrapper";

    // Metadata-identifier that marks generated functions that wrap ghost expressions
    const std::string PALLAS_GHOST_WRAPPER_MARKER = "pallas.ghostWrapper";

    // Identifier to mark predicate definitions
    const std::string PALLAS_PREDICATE_DEF_MARKER = "pallas.predDef";

    // Identifier to mark ghost methods
    const std::string PALLAS_GHOST_MARKER = "pallas.ghost";

    // String that is used to identify a MD-Tuple as a source-location
    const std::string PALLAS_SRC_LOCATION_MARKER = "pallas.srcLoc";

    /* Constants used to name specification-functions */
    const std::string PALLAS_SPEC_IMPLY = "pallas.imply";

    const std::string PALLAS_SPEC_SCAND = "pallas.scAnd";

    const std::string PALLAS_SPEC_SCOR = "pallas.scOr";

    const std::string PALLAS_SPEC_SEPCONJ = "pallas.sepConj";

    const std::string PALLAS_SPEC_OLD_BASE = "pallas.old";

    const std::string PALLAS_SPEC_RESULT_BASE = "pallas.result";

    const std::string PALLAS_SPEC_BOUND_VAR = "pallas.boundVar";

    const std::string PALLAS_SPEC_EXISTS = "pallas.exists";

    const std::string PALLAS_SPEC_FORALL = "pallas.forall";

    const std::string PALLAS_SPEC_FORALL_SEP = "pallas.forallSep";

    const std::string PALLAS_SPEC_FRAC_OF = "pallas.fracOf";

    const std::string PALLAS_SPEC_PERM = "pallas.perm";

    const std::string PALLAS_SPEC_PTR_BLOCK_LENGTH = "pallas.ptrBlockLength";

    const std::string PALLAS_SPEC_PTR_BLOCK_OFFSET = "pallas.ptrBlockOffset";

    const std::string PALLAS_SPEC_PTR_LENGTH = "pallas.ptrLength";

    const std::string PALLAS_SPEC_UNFOLDING_BASE = "pallas.unfolding";

    const std::string PALLAS_SPEC_SEQ_NEW_BASE = "pallas.seq.new";

    const std::string PALLAS_SPEC_SEQ_SIZE_BASE = "pallas.seq.size";

    const std::string PALLAS_SPEC_SEQ_GET_BASE = "pallas.seq.get";

    const std::string PALLAS_SPEC_SEQ_PREPEND_BASE = "pallas.seq.prepend";

    const std::string PALLAS_SPEC_SEQ_EQUALS_BASE = "pallas.seq.equals";

    const std::string PALLAS_SPEC_SEQ_SLICE_BASE = "pallas.seq.slice";

    const std::string PALLAS_SPEC_SEQ_SET_BASE = "pallas.seq.set";


    /* Constants used to name specification-types */
    const std::string PALLAS_SPEC_TYPE_FRAC = "pallas.fracT";

    const std::string PALLAS_SPEC_TYPE_SEQ_BASE = "pallas.seq";


}

#endif // PALLAS_IRSPECCONSTANTS_H