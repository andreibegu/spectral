# C++ Specification-Library
This document provides an overview of the encoding of specifications in the C++-frontend of the framework.
The implementation of the specification-library can be found [here](/res/spec_libs/cpp/cpp_spec_lib.h).

The functions in the C++ specification-library are located in the namespace ```pallasSpec```. 

To avoid having to specify the namespace for every call to a function from the library, one can include the following declares-clause:
```
/*@
declare using namespace pallasSpec;
@*/
```

## Operators:
- Implication: ```_imply(a, b)```
- Short-circuiting conjunction: ```_and(a, b)```
- Short-circuiting disjunction: ```_or(a, b)```
- Separating conjunction: ```_sep(a, b)```

In general, the usual logical operators of C++ can be used in specifications. 
Due to some limitations of the verification-support for VerCors, 
short-circuiting operators are not supported within quantifiers. In these cases, ```_and``` and ```_or``` can be used instead.

## Specification-constructs:

### Old-value of an expression
Reference to the value of expression ```e``` before the function was evaluated:
```
_old<T>(e)
```
Where ```T``` is the type of the expression.

### Return-value of a function
Represents the the return-value of a function:
```
_result<T>()
```

Where ```T``` is the return-type of the function.
Can only  be used in ensures-clauses. 

### Reference to the return-value of a function
Represents a const-reference to the return-value of a function:
```
_resultRef<T>()
```

Where ```T``` is the return-type of the function.
Can only be used in ensures-clauses. 

Compared to ```_result```, this can be useful when a function returns class- or 
struct-types with a non-trivial destructor. 
For these ```_result``` would insert a destructor-call into the specification, making the specification no longe side-effect free.

### Quantifiers
Quantifiers are expressed by calling 
- ```_forall(range, expr)```: Universal quantifier
- ```_forallS(range, expr)```: Separating universal quantifier
- ```_exists(range, expr)```: Existential quantifier

```range``` is an expression that specifies bound on the used bound-variables and 
```expr``` is the quantified expression. 

Bound variables are introduced by calling ```_bv<T>(ID)``` where ```T``` is the type of the variable and ```ID``` is a string-literal that identifies the variable. 
Bound variables are implicitly declared based on the calls to ```_bv``` within ```range```.

### Asserting Permissions
Assert fractional permission over a location: 
```
_Perm(LOC, _fracOf(X, Y))
```
Where: 
- ```LOC``` is a pointer
- ```_fracOf(X, Y)``` encodes a fraction of ```X \ Y```.
- Fractions are represented by the type ```frac```

### Pointer-properties
The VerCors-constructs for specifying properties about pointers are encoded as follows: 
- ```\pointer_length(p)```: ```_ptrLength(p)```
- ```\pointer_block_length(p)```: ```_ptrBlockLength(p)```
- ```\pointer_block_offset(p)```: ```_ptrBlockOffset(p)```

### Predicates
To temporarily unfold a predicate while evaluating an expression:
```
_unfolding<T>(PRED, EXPR)
```
where ```T``` is the type of the expression, ```PRED``` is the predicate application and ```EXPR``` is the expression.

### Sequence-ADT
A sequence containing elements of type ```T``` is represented by the struct-type ```seq<T>```
For this type, the following operations are available:
- Create an empty sequence: ```_seqNew<T>()```
- Size of a sequence: ```_seqSize<T>(seq)```
- Access element at index: ```_seqGet<T>(seq, idx)```
- Prepend element to sequence: ```_seqPrepend<T>(elem, seq)```
- Equality of sequences: ```_seqEq<T>(seq1, seq2)```
- Slice of a sequence: ```_seqSlice<T>(seq, startIdx, endIdx)```
    - Including start-index
    - Excluding end-index
- Update element of a sequence: ```_seqSet<T>(seq, idx, value)```

## Convenience-Macros:
The following macros are available to make recurring patterns in specifications less verbose:
- ```_writePerm``` --> Fractional permission of ```1```
- ```_inRange(low, v, up)``` --> ```v``` is in the interval ```[low ; up)]```
- ```_seqHead(s)``` --> Head-element of sequence `s`
- ```_seqTail(s)``` --> Tail of sequence `s`
