parser grammar SpecParser;
options { tokenVocab = SpecLexer; }

file: nonSpec (specDeclarationBlock nonSpec)? (spec nonSpec)*;

spec
    : specContractBlock 
    | specStatementBlock 
    | specLoopInvariantBlock 
    | wrapperLocationMarker
    | predicateDefBlock;

specDeclarationBlock: SPEC_START specDeclaration+ SPEC_END;

specContractBlock: SPEC_START specMappingClause? specPureFlag? specContractClause+ SPEC_END;

specStatementBlock: SPEC_START specStatement+ SPEC_END;

specLoopInvariantBlock: SPEC_START specLoopInvariant+ SPEC_END;

wrapperLocationMarker: SPEC_START 'wrapper location' specExpression? SPEC_EXPR_END SPEC_END;

predicateDefBlock: SPEC_START predicateDef+ SPEC_END;

specContractClause
    : ('requires' specExpression SPEC_EXPR_END) # requiresClause
    | ('ensures'  specExpression SPEC_EXPR_END) # ensuresClause
    ;

specMappingClause
    : 'assumed'? 'contract' 'for' ('external' | 'generated') ('ir' | 'src')? specExpression SPEC_EXPR_END wrapperSigClause?;

/* Specification of signature for external contracts */
wrapperSigClause: wrapperArgClause wrapperRetClause;
wrapperArgClause: 'args' wrapperArgList? SPEC_EXPR_END;
wrapperArgList
    :  argExpr
    | (argExpr WRAPPER_ARG_SEP wrapperArgList)
    ;
wrapperRetClause: 'returns' specExpression SPEC_EXPR_END;

specPureFlag
    : 'pure' SPEC_EXPR_END # pureFlag;

specLoopInvariant
    : ('loop_invariant' specExpression SPEC_EXPR_END)
    ;

specStatement
    : ('assert' specExpression SPEC_EXPR_END) # assertStatement
    | ('assume' specExpression SPEC_EXPR_END) # assumeStatement
    ;

specDeclaration
    : ('declare' specExpression SPEC_EXPR_END) # declarationClause
    ;

predicateDef: 'predicate' predicateSig ':=' specExpression SPEC_EXPR_END ;

predicateSig: (~PRED_DEF)+;
argExpr: (~WRAPPER_ARG_SEP | SPEC_EXPR_END)+ ; 
specExpression: (~SPEC_EXPR_END)+ ;

nonSpec: (~(SPEC_START | SPEC_END))*;