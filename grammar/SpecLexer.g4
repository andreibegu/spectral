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
Whitespace: [ \t]+ -> channel(HIDDEN);
Newline: ( '\r' '\n'? | '\n') -> channel(HIDDEN);
SPEC_OTHER: . ;
