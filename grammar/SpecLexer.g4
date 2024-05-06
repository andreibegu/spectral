lexer grammar SpecLexer;

// Default-mode 
SPEC_START: '/*@' -> mode(SPEC);
OTHER: . -> skip;


// Specification-mode
mode SPEC;
SPEC_END: '@*/' -> mode(DEFAULT_MODE); 

REQUIRES: 'requires';
ENSURES: 'ensures';
ASSERT: 'assert';
ASSUME: 'assume';
LOOP_INVARIANT: 'loop_invariant';

SEMI: ';';
Whitespace: [ \t]+ -> skip; // channel(HIDDEN);
Newline: ( '\r' '\n'? | '\n') -> skip; // channel(HIDDEN);
SPEC_OTHER: . ;

// EXPR: [a-zA-Z_$] [a-zA-Z_$0-9]* ;
// EXPR: (~';')+ ;




/*
@lexer::members {
    private: 
		bool inBlockSpec = false;
    	bool inLineSpec = false;
}

// Default-mode (source, no specification)
StartBlockSpec:
	'/*' [ \t\u000C]* '@' {inBlockSpec = true;} -> mode(SPEC);
StartLineSpec:
	'//' [ \t\u000C]* '@' {inLineSpec = true;} -> mode(SPEC);

Other: . -> skip; // channel(HIDDEN);


// Specification mode
mode SPEC;
Requires: 'requires';
Ensures: 'ensures';
LoopInv: 'loop_invariant';
Assert: 'assert';
Assume: 'assume';

Semi: ';';
SPEC_CONTENT: . ;
 */

//EndSpec:
//	(
//		'@'? '*/' {inBlockSpec}? {inBlockSpec = false;}
//		| ('\n' | '\r\n') {inLineSpec}? {inLineSpec = false;}
//	) -> mode(DEFAULT_MODE);

//Whitespace: [ \t]+ -> skip; // channel(HIDDEN);
//Newline: ( '\r' '\n'? | '\n') -> skip; // channel(HIDDEN);

