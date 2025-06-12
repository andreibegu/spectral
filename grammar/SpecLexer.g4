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
DECLARE: 'declare';
LOOP_INVARIANT: 'loop_invariant';
PURE: 'pure';
EXTERNAL: 'external';
CONTRACT: 'contract';
ASSUMED: 'assumed';
IR: 'ir';
SRC: 'src';
WRAPPER_LOC: 'wrapper location';

SPEC_EXPR_END: ';';
Whitespace: [ \t]+ -> channel(HIDDEN);
Newline: ( '\r' '\n'? | '\n') -> channel(HIDDEN);
SPEC_OTHER: . ;
