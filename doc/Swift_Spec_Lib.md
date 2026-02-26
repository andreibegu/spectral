# Swift Specification-Library
This document provides an overview of the encoding of specifications in the Swift-frontend of the framework.

A part of the specification-library is implemented [here](/res/spec_libs/swift/PallasSpec/Sources/PallasSpec/PallasSpec.swift).
Other parts of the specification-library are generated during the [wrapper-generation stage](/lib/swift/SwiftWrapperGenerator.cpp).

## Operators:
- Implication: ```a ==> b```
- Short-circuiting conjunction: ```&&&```
- Short-circuiting disjunction: ```|||```
- Separating conjunction: ```**```

In general, the usual logical operators of Swift can be used in specifications. 
Due to some limitations of the verification-support for VerCors, 
short-circuiting operators are not supported within quantifiers. In these cases, the custom ```&&&``` and ```|||``` operators can be used instead.

## Specification-constructs:

### Old-value of an expression
Reference to the value of expression ```e``` before the function was evaluated:
```
_old(e)
```

### Return-value of a function
Reference to the return-value of a function:
```
_result()
```

Only valid in ensures-clauses. 

### Quantifiers
Quantifiers are expressed by calling 
- ```_forall(range, expr)```: Universal quantifier
- ```_forallSep(range, expr)```: Separating universal quantifier
- ```_exists(range, expr)```: Existential quantifier

```range``` is an expression that specifies bound on the used bound-variables and 
```expr``` is the quantified expression. 

Bound variables are introduced by calling ```_bvT(ID)``` where ```T``` is the type of the variable and ```ID``` is a string-literal that identifies the variable. 
The type of the bound variable is encoded in the name of the function, i.e. to refer to a bound variable 'x' of type 'Int', the call looks like ```_bvInt("x")```.
Bound variables are implicitly declared based on the reference bound variables within the ```range```-expression.

### Asserting Permissions
Assert fractional permission over a location: 
```
_perm(LOC, Frac(X, Y))
```
Where: 
- ```LOC``` is a reference
- ```Frac(X, Y)``` encodes a fraction of ```X \ Y```.
- Fractions are represented by the type ```Frac```

### Predicates
Temporarily unfold a predicate while evaluating an expression:
```
_unfoldingT(PRED, EXPR)
``` 
where ```T``` is the type of the variable, ```PRED``` is the predicate and ```EXPR``` is the expression.

If the type of the expression is ```Bool```, the type-suffix can be omitted: 
```
_unfolding(PRED, EXPR)
``` 

### Sequence-ADT
To use sequences that contain elements of type ```T```, the following declares-clause must be added:
```
/*@
declare #defSeq<AStruct>;
@*/
```

A sequence containing elements of type ```T``` is then represented by the type ```SeqT``` (i.e. the content-type of the sequence is encoded in the sequence's name).
For this type, the following operations are available:
- Create an empty sequence: ```SeqT()```
- Size of a sequence: ```seq.size()```
- Access element at index: ```seq[idx]```
- Prepend element to sequence: ```seq.prepend(elem)```
- Equality of sequences: ```seq1.equals(seq2)```

Where ```seq```, ```seq1``` and ```seq2``` are sequences of type ```SeqT```.

