parser grammar SpecParser;
options { tokenVocab = SpecLexer; }

file: nonSpec (specDeclarationBlock nonSpec)? (spec nonSpec)*;

spec: specContractBlock | specStatementBlock | specLoopInvariantBlock;

specDeclarationBlock: SPEC_START specDeclaration+ SPEC_END;

specContractBlock: SPEC_START specExternalClause? specPureFlag? specContractClause+ SPEC_END;

specStatementBlock: SPEC_START specStatement+ SPEC_END;

specLoopInvariantBlock: SPEC_START specLoopInvariant+ SPEC_END;

specContractClause
    : ('requires' specExpression SPEC_EXPR_END) # requiresClause
    | ('ensures'  specExpression SPEC_EXPR_END) # ensuresClause
    ;

specExternalClause
    : ('external' 'assumed'? 'contract' ('ir' | 'src')? specExpression SPEC_EXPR_END );

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

specExpression: (~SPEC_EXPR_END)+ ;

nonSpec: (~(SPEC_START | SPEC_END))*;