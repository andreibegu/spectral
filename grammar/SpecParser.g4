parser grammar SpecParser;
options { tokenVocab = SpecLexer; }

file: .*? (specDeclarationBlock .*?)? (spec .*?)*;

spec: specContractBlock | specStatementBlock | specLoopInvariantBlock;

specDeclarationBlock: SPEC_START specDeclaration+ SPEC_END;

specContractBlock: SPEC_START specContractClause+ SPEC_END;

specStatementBlock: SPEC_START specStatement+ SPEC_END;

specLoopInvariantBlock: SPEC_START specLoopInvariant+ SPEC_END;

specContractClause
    : ('requires' specExpression ';') # requiresClause
    | ('ensures'  specExpression ';') # ensuresClause
    ;

specLoopInvariant
    : ('loop_invariant' specExpression ';')
    ;

specStatement
    : ('assert' specExpression ';') # assertStatement
    | ('assume' specExpression ';') # assumeStatement
    ;

specDeclaration
    : ('declare' specExpression ';') # declarationClause
    ;

specExpression: (~';')+ ;