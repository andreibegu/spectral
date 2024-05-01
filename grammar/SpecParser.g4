parser grammar SpecParser;
options { tokenVocab = SpecLexer; }

file: .*? (spec .*?)+;

spec: specStatementBlock | specContractBlock;

specContractBlock: startSpec specContractClause* endSpec;

specStatementBlock: startSpec specStatement* endSpec;

specContractClause: 'requires' Expr ';' | 'ensures' Expr ';' | 'loop_invariant' Expr ';';

specStatement: 'assert' Expr ';' | 'assume' Expr ';';


startSpec: StartLineSpec | StartBlockSpec;

endSpec: EndSpec;