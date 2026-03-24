# C Specification-Library
This document provides an overview of the encoding of specifications in the C-frontend of the framework.
The implementation of the specification-library can be found [here](/res/spec_libs/c/c_spec_lib.h).

## Operators:
- Implication: ```_imply(a, b)```
- Short-circuiting conjunction: ```_and(a, b)```
- Short-circuiting disjunction: ```_or(a, b)```
- Separating conjunction: ```_sep(a, b)```

In general, the usual logical operators of C can be used in specifications. 
Due to some limitations of the verification-support for VerCors, 
short-circuiting operators are not supported within quantifiers. In these cases, ```_and``` and ```_or``` can be used instead.

## Specification-constructs:

### Old-value of an expression
Reference to the value of expression ```e``` before the function was evaluated:
```_old(T)(e)```

Where ```T``` is the type of the expression. 
Requires ```DEF_OLD(T)``` to be called in a declares-clause to declare the required function. 

### Return-value of a function
Reference to the return-value of a function:
```_result(T)```

Where ```T``` is the return-type of the function. 
Requires ```DEF_RESULT(T)``` to be called in a declares-clause to declare the required function. 
Can only  be used in ensures-clauses. 

### Quantifiers
Quantifiers are expressed by calling 
- ```_forall(range, expr)```: Universal quantifier
- ```_forallS(range, expr)```: Separating universal quantifier
- ```_exists(range, expr)```: Existential quantifier

```range``` is an expression that specifies bound on the used bound-variables and 
```expr``` is the quantified expression. 

Bound variables are introduced by calling ```_bv(T, ID)``` where ```T``` is the type of the variable and ```ID``` an identifier. 
Bound variables are implicitly declared based on the calls to ```_bv``` within ```range```.
To call ```_bv(T, ID)```, a call to ```DEF_BV(T)``` must be made within a declares-clause. 

### Asserting Permissions
To assert a fractional-permission over a location, a call to 
```_Perm(LOC, _fracOf(X, Y))``` is used, where ```LOC``` is a pointer. 

```_fracOf(X, Y)``` encodes a fraction of ```X \ Y```.

### Pointer-properties
The VerCors-constructs for specifying properties about pointers are encoded as follows: 
- ```\pointer_length(p)```: ```_ptr_length(p)```
- ```\pointer_block_length(p)```: ```_ptr_block_length(p)```
- ```\pointer_block_offset(p)```: ```_ptr_block_offset(p)```

### Predicates
To temporarily unfold a predicate while evaluating an expression, a call to ```_unfolding(T)(PRED, EXPR)``` is used where ```PRED``` is the predicate, ```EXPR``` is the expression and ```T``` is the type of the expression.

Requires ```DEF_UNFOLDING(T)``` to be called in a declares-clause to declare the required function. 


### Sequence-ADT
A sequence containing elements of type ```T``` is represented by the struct-type ```SEQ(T)```
For this type, the following operations are available:
- Create an empty sequence: ```_seqNew(T)()```
- Size of a sequence: ```_seqSize(T)(seq)```
- Access element at index: ```_seqAt(T)(seq, idx)```
- Prepend element to sequence: ```_seqPrepend(T)(elem, seq)```
- Equality of sequences: ```_seqEq(T)(seq1, seq2)```
- Slice of a sequence: ```_seqSlice(T)(seq, startIdx, endIdx)```
    - Including start-index
    - Excluding end-index
- Update element of a sequence: ```_seqSet(T)(seq, idx, value)```

To use sequences with content-type ```T```, a call to ```DEF_SEQ(T)``` in a declares-clause is required. 

## Convenience-Macros:
The following macros are available to make recurring patterns in specifications less verbose:
- ```_write``` --> Fractional permission of ```1```
- ```_inRange(low, v, up)``` --> ```v``` is in the interval ```[low; up)]```
- ```_seqHead(type, s)``` --> Head-element of sequence `s`
- ```_seqTail(type, s)``` --> Tail of sequence `s`