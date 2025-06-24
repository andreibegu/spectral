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
ASSUMED: 'assumed';
CONTRACT: 'contract';
FOR: 'for';
EXTERNAL: 'external';
GENERATED: 'generated';
IR: 'ir';
SRC: 'src';
WRAPPER_LOC: 'wrapper location';
ARGS: 'args';
RETURNS: 'returns';

WRAPPER_ARG_SEP: ',';
SPEC_EXPR_END: ';';
Whitespace: [ \t]+ -> channel(HIDDEN);
Newline: ( '\r' '\n'? | '\n') -> channel(HIDDEN);
SPEC_OTHER: . ;
