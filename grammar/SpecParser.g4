parser grammar SpecParser;
options { tokenVocab = SpecLexer; }

file: .*? (spec .*?)*;

spec: specContractBlock | specStatementBlock ;

specContractBlock: SPEC_START specContractClause+ SPEC_END;

specStatementBlock: SPEC_START specStatement+ SPEC_END;

specContractClause
    : ('requires' specExpression ';') # requiresClause
    | ('ensures'  specExpression ';') # ensuresClause
    | ('loop_invariant' specExpression ';') # invariantClause
    ;

specStatement
    : ('assert' specExpression ';') # assertStatement
    | ('assume' specExpression ';') # assumeStatement
    ;

specExpression: (~';')+ ;