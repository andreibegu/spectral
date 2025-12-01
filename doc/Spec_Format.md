# Source Level Specification Format 

This document outlines the structure of the source-level specification format. 

In general, the high-level structure of the specification format is language independent and reused for all supported languages. 
The high-level specifications are completed with expressions which use the syntax of the used source language. 
Additionally, every suported source language provides a 'specification library' which provies functions and types to encode specification-specific constructs (e.g. `_result()` to refer to the value returned from a function).

## Specification comments
High-level specifications must be placed in special comments that have the following shape: 
```
/*@
SPECIFCATIONS GO HERE
@*/
```

In general it is recommended to place specificaion comments on separate lines and not on the same line as e.g. staements in the source-program. 

**NOTE:** The following kinds of specification-comments are not supported: 
```
/*@
THIS IS NOT VALID
*/  <-- The '@' is missing
```
```
//@ THIS IS NOT VALID
```

Additionally, it is not  allowed that two specification-blocks are associated with the same LLVM construct (e.g. a function or instruction). 
This is to make potential errors during the matching step easier to detect. 

E.g. the following is not allowed:
```
/*@
    CONTRACT_1
@*/

/*@
    CONTRACT_2
@*/

int foo(int a, int b) {
    [...]
}
```

Instead, one can merge the separate specification-blocks into a single block:
```
/*@
    CONTRACT_1
    CONTRACT_2
@*/

int foo(int a, int b) {
    [...]
}
```

## Function Contracts
Function contracts are written within specification comments using the following grammar (where `EXPR` is an expression from the source language that does not contain `;`):
```
CONTRACT     ::= '/*@' 'pure;'? 
                        CONTR_CLAUSE+ 
                   '@*/'
CONTR_CLAUSE ::= ('requires' | 'ensures') EXPR ';'
```

Function contracts can start with an optional 'pure'-flag which indicates that the annotated function is side-effect free.
A contract further consists of a number of `requires`- and `ensures`-clauses that specify the pre- and post-conditions of the annotated function.

Function contracts are matched to the first function below their definition.  
In general it is advisable, to place the function contract on a separate line above the definition of the function that they refer to (instead of placing them on the same line of the funciton definition).


## Loop Invariants
Loop invariants are written within specification comments using the following grammar (where `EXPR` is an expression from the source language that does not contain `;`):
```
LOOP_INV   ::= '/*@' INV_CLAUSE+ '@*/'
INV_CLAUSE ::= 'loop-invariant' EXPR ';'
```

Loop invariants are matched to the first loop in the LLVM-IR below their definition and must be placed before the loop header in the source file. 

## Specification Statements
Blocks of specification statements are written within specification comments using the following grammar (where `EXPR` is an expression from the source language that does not contain `;`) and can contain a number of specification statements:

```
STMNT_BLOCK ::= '/*@' STMNT+ '@*/'
STMNT       ::= ('assert' | 'assume' | 'fold' | 'unfold') EXPR ';'
```

The `assert` and `assume` statements can be used to check or assume properties in the program. 
The `fold` and `unfold` statements are used to fold and unfold predicates. Hence the expression must be a predicate for the latter two types of statements.

Statements are matched to the first instruction in the LLVM-IR that contains a debug-location and occurs after the definition of the specification statement. 
To avoid that blocks of specification statements are matched to instructions in an unintuitive way, they should always be specified above a statement in the used source-language. 

E.g. the following example of a C program shows a good placement of an assert-statement because it is placed directly in front of a statement in the C-program:
```
int abs(int a) {
    int res = a;
    if (a < 0) {
        /*@ assert a >= 0; @*/
        res = -res;
    }
    return res;
}
```

The next example on the other hand shows a placement of an assert statement where the mapping of the statement is not clear:
```
int abs(int a) {
    int res = a; 
    if (a < 0) {
        res = -res;
        /*@ assert a >= 0; @*/
    }
    return res;
}
```
In this case the mapping of the assert-statement to the LLVM-instruction is less intuitive. 
I.e. the assert would be mapped to the jump-instruction at the end of the if-block which is not visible in the C program and can cause unintuitive error messages. 
Therefor this should be avoided. 

## Predicate Definitions
Predicates are defined in specification comments using the following syntax:
```
PRED_BLOCK ::= '/*@  PRED_DEF+ @*/'
PRED_DEF   ::= 'predicate' NAME '(' ARGS ')' ':=' EXPR ';'
```
Where 
- `NAME` is a character sequence that does not contain `(` and that specifies the name of the predicate. Moreover, `NAME` must be a valid function name from the used source language.
- `ARGS` is a character sequence that does not contain `)` and that specifies the arguments of the predicate. The concrete format of `ARGS` can be determined by the used source language, but in general it should match the syntax of an argument list in the source language.
- `EXPR` is an expression from the source language that does not contain `;`

Predicate definitions are inserted into the source in the location where thay are defined. 
Hence the location where predicates are define matters as all used types must be in scope. 

## External & Generated Contracts
**TODO: Write documentation for this!**

## Specification Expressions
Expressions that are used in the high-level specification costructs (`EXPR` in the examples above) use the syntax of the used source language and must generally return a boolan when used in the high-level specification constructs.

To encode specification specific constructs, each supported lamguage provides a specification library:
- [C specification library](doc/C_Spec_Lib.md)
- [C++ specification library](doc/Cpp_Spec_Lib.md)
- [Swift specification library](doc/Swift_Spec_Lib.md)