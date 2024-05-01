lexer grammar SpecLexer;

@lexer::members {
    private: 
		bool inBlockSpec = false;
    	bool inLineSpec = false;
}

/*
 * Default-mode (source, no specification)
 */
StartBlockSpec:
	'/*' [ \t\u000C]* '@' {inBlockSpec = true;} -> mode(SPEC);
StartLineSpec:
	'//' [ \t\u000C]* '@' {inLineSpec = true;} -> mode(SPEC);

// Drop everything that is not within a spec-block
Other: . -> channel(HIDDEN);
Whitespace: [ \t]+ -> channel(HIDDEN);
Newline: ( '\r' '\n'? | '\n') -> channel(HIDDEN);

/*
 * Specification mode
 */
mode SPEC;
Requires: 'requires';
Ensures: 'ensures';
LoopInv: 'loop_invariant';
Assert: 'assert';
Assume: 'assume';

Semi: ';';
Expr: (~';')+ ;

EndSpec:
	(
		'@'? '*/' {inBlockSpec}? {inBlockSpec = false;}
		| ('\n' | '\r\n') {inLineSpec}? {inLineSpec = false;}
	) -> mode(DEFAULT_MODE);