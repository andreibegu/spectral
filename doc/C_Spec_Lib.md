# C Specification-Library
This document provides an overview of the encoding of specifications in the C-frontend of the framework.

## Operators:
- Implication: ```_imply(a, b)```
- Short-circuting conjunction: ```_and(a, b)```
- Short-circuting disjunction: ```_or(a, b)```
- Separating conjunction: ```_sep(a, b)```

In general, the usual logical operators of C can be used in specifications. 
Due to some limitations of the verification-support for VerCors, 
short-circuting operators are not supported within quantifiers. In these cases, ```_and``` and ```_or``` can be used instead.

## Specification-constructs:

### Old-value of an expression
Reference to the value of expression ```e``` before the function was evaluated:
```_old(T)(e)```

Where ```T``` is the type of the expression. 
Requires ```DEF_OLD(T)``` to be called in a declares-clause to declare the required function. 

### Return-value of a function
Reference to the return-value of a function:
```_result(T)```

Where ```T``` is the type of the expression. 
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


### Sequence-ADT
A sequence containing elements of type ```T``` is representd by the struct-type ```SEQ(T)```
For this type, the following operations are available:
- Create an empty sequence: ```_seqNew(T)()```
- Size of a sequence: ```_seqSize(T)(seq)```
- Access element at index: ```_seqAt(T)(seq, idx)```
- Prepend element to sequence: ```_seqPrepend(T)(elem, seq)```
- Equality of sequences: ```_seqEq(T)(seq1, seq2)```

To use sequences with content-type ```T```, a call to ```DEF_SEQ(T)``` in a declares-clause is required. 