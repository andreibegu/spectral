parser grammar SpecParser;
options { tokenVocab = SpecLexer; }

file: .*? (spec .*?)*;

spec: spec_contract_block | spec_statement_block ;

spec_contract_block: SPEC_START spec_contract_clause+ SPEC_END;

spec_statement_block: SPEC_START spec_statement+ SPEC_END;

spec_contract_clause
    : ('requires' spec_expression ';')
    | ('ensures'  spec_expression ';')
    | ('loop_invariant' spec_expression ';')
    ;

spec_statement
    : ('assert' spec_expression ';')
    | ('assume' spec_expression ';')
    ;

spec_expression: (~';')+ ;

/*
file: .*? (spec .*?)* EOF;

spec: specStatementBlock | specContractBlock;

specContractBlock: startSpec specContractClause+ endSpec;

specStatementBlock: startSpec specStatement+ endSpec;

specContractClause: 'requires' (~';')+ ';' | 'ensures' (~';')+ ';' | 'loop_invariant' (~';')+ ';';

specStatement: 'assert' (~';')+ ';' | 'assume' (~';')+ ';';


startSpec: StartLineSpec | StartBlockSpec;

endSpec: EndSpec;

*/