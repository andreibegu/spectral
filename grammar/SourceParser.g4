grammar SourceParser;

@lexer::members {
    private static boolean inBlockSpec = false;
    private static boolean inLineSpec = false;
}

file: .*? (spec .*?)+;

spec: specStatementBlock | specContractBlock;

specContractBlock: startSpec specContractClause* endSpec;

specStatementBlock: startSpec specStatement* endSpec;

specContractClause: 'requires' expr ';' | 'ensures' expr ';' | 'loop_invariant' expr ';';

specStatement: 'assert' expr ';' | 'assume' expr ';';

expr: (~';')+;

startSpec: StartLineSpec | StartBlockSpec;

endSpec: EndSpec;



StartBlockSpec:
	'/*' [ \t\u000C]* '@' {inBlockSpec = true;};
StartLineSpec:
	'//' [ \t\u000C]* '@' {inLineSpec = true;};

Requires: 'requires';
Ensures: 'ensures';
LoopInv: 'loop_invariant';
Assert: 'assert';
Assume: 'assume';

Semi: ';';

EndSpec:
	'@'? '*/' {inBlockSpec}? {inBlockSpec = false;}
	| ('\n' | '\r\n') {inLineSpec}? {inLineSpec = false;}
	;

Whitespace: [ \t]+ -> channel(HIDDEN);
Newline: ( '\r' '\n'? | '\n') -> channel(HIDDEN);
Other: . ;