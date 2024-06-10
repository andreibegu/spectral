parser grammar SpecParser;
options { tokenVocab = SpecLexer; }

file: .*? (specDeclarationBlock .*?)? (spec .*?)*;

spec: specContractBlock | specStatementBlock | specInvariantBlock;

specDeclarationBlock: SPEC_START specDeclaration+ SPEC_END;

specContractBlock: SPEC_START specContractClause+ SPEC_END;

specStatementBlock: SPEC_START specStatement+ SPEC_END;

specInvariantBlock: SPEC_START specInvariant+ SPEC_END;

specContractClause
    : ('requires' specExpression ';') # requiresClause
    | ('ensures'  specExpression ';') # ensuresClause
    ;

specInvariant
    : ('loop_invariant' specExpression ';') # invariantClause
    ;

specStatement
    : ('assert' specExpression ';') # assertStatement
    | ('assume' specExpression ';') # assumeStatement
    ;

specDeclaration
    : ('declare' specExpression ';') # declarationClause
    ;

specExpression: (~';')+ ;