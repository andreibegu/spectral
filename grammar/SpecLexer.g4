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
FOLD: 'fold';
UNFOLD: 'unfold';
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
GIVEN: 'given';
YIELDS: 'yields';
PREDICATE: 'predicate';
PRED_DEF: ':=';
INLINE: 'inline';
GHOST: 'ghost';
METHOD: 'method';
FUNCTION: 'function';
EQUALS: '=';
ASSIGN: 'assign';


PRED_SIG_START: '(';
PRED_SIG_END: ')';
WRAPPER_ARG_SEP: ',';
SPEC_EXPR_END: ';';
Whitespace: [ \t]+ -> channel(HIDDEN);
Newline: ( '\r' '\n'? | '\n') -> channel(HIDDEN);
SPEC_OTHER: . ;
