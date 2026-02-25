# Source Level Specification Format 

This document outlines the structure of the source-level specification format. 

In general, the high-level structure of the specification format is language independent and reused for all supported languages. 
The high-level specifications are completed with expressions which use the syntax of the used source language. 
Additionally, every supported source language provides a 'specification library' which provides functions and types to encode specification-specific constructs (e.g. `_result()` to refer to the value returned from a function).

## Specification comments
High-level specifications must be placed in special comments that have the following shape: 
```
/*@
SPECIFCATIONS GO HERE
@*/
```

In general it is recommended to place specification comments on separate lines and not on the same line as e.g. statements in the source-program. 

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
Function contracts are written within specification comments using the following grammar (where `EXPR` is an expression from the source language that does not contain `;` and `GIVEN_DEF` and `YIELDS_DEF` are definitions of ghost arguments as outlined in section 'Ghost Arguments'):
```
CONTRACT     ::= '/*@' 'pure;'? 
                        GIVEN_DEF*
                        YIELDS_DEF*
                        CONTR_CLAUSE+ 
                   '@*/'
CONTR_CLAUSE ::= ('requires' | 'ensures') EXPR ';'
```

Function contracts can start with an optional 'pure'-flag which indicates that the annotated function is side-effect free.
Additionally, a contract can optionally define ghost arguments that are passed into the function (given-arguments) and ghost arguments that are returned from the function (yields-arguments).
The definition and usage of these ghost arguments is outlined in more detail in section *Ghost Arguments* below.
A contract further consists of a number of `requires`- and `ensures`-clauses that specify the pre- and post-conditions of the annotated function.

Function contracts are matched to the first function below their definition.  
In general it is advisable, to place the function contract on a separate line above the definition of the function that they refer to (instead of placing them on the same line of the function definition).


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
STMNT       ::= (('assert' | 'assume' | 'fold' | 'unfold') EXPR ';') |
                  GHOST_ASSIGN
```

The `assert` and `assume` statements can be used to check or assume properties in the program. 
The `fold` and `unfold` statements are used to fold and unfold predicates. Hence the expression must be a predicate for the latter two types of statements.

In the grammar above, `GHOST_ASSIGN` represents an assignment to a ghost variable. This is described further in section *Ghost Assignments*.

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
Therefore this should be avoided. 

## Predicate Definitions
Predicates are defined in specification comments using the following syntax:
```
PRED_BLOCK ::= '/*@  PRED_DEF+ @*/'
PRED_DEF   ::= 'inline'? 'predicate' NAME '(' ARGS ')' ':=' EXPR ';'
```
Where 
- `NAME` is a character sequence that does not contain `(` and that specifies the name of the predicate. Moreover, `NAME` must be a valid function name from the used source language.
- `ARGS` is a character sequence that does not contain `)` and that specifies the arguments of the predicate. The concrete format of `ARGS` can be determined by the used source language, but in general it should match the syntax of an argument list in the source language.
- `EXPR` is an expression from the source language that does not contain `;`

Predicate definitions are inserted into the source in the location where they are defined. 
Hence the location where predicates are define matters as all used types must be in scope.

When the `inline` keyword is given, the predicate is treated as an inline-predicate whose definition is inlined wherever the predicate occurs.

## External & Generated Contracts
Regular contracts are attached directly to the definition of a functions.
To abstract the behavior of functions that are defined and imported from external files (e.g. library functions) or generated by the compiler (e.g. generated constructors in C++), external and generated contracts can be used. 

Within external and generated contracts, the arguments of the function to which they are applied are accessible. 
Other variables (e.g. global variables) are not accessible.

Contracts for external or generated functions are specified using the following grammar:
```
EXT_CONTR       ::= '/*@' 'assumed'? 'contract' 'for' 'external'
                           MAPPING_CLAUSE
                           SIG_CLAUSE?
                          'pure;'? 
                           GIVEN_DEF*
                           YIELDS_DEF*
                           CONTR_CLAUSE+
                      '@*/'

GEN_CONTR       ::= '/*@' 'assumed'? 'contract' 'for' 'generated'
                           MAPPING_CLAUSE
                          'pure;'? 
                           GIVEN_DEF*
                           YIELDS_DEF*
                           CONTR_CLAUSE+
                      '@*/'


MAPPING_CLAUSE  ::= ('ir' | 'src')? EXPR ';'

SIG_CLAUSE      ::= 'args' ARG_LIST? ';'
                    'returns' EXPR ';'

ARG_LIST        ::= ARG_EXPR | (ARG_EXPR ',' ARG_LIST) 
```
where 
- `GIVEN_DEF` is a definition of a given-argument (see section Ghost Arguments)
- `YIELDS_DEF` is a definition of a yields-argument (see section Ghost Arguments)
- `EXPR` is an expression from the source language that does not contain `;`
- `CONTR_CLAUSE` is a contract clause with the same grammar as in a 'normal' contract
- `ARG_EXPR` is an arbitrary string from the source language that does not contain ',' or ';'. In practice, this contains definitions of function arguments. 

The base structure of external and generated contracts is similar to that of a regular function contract in that they also contain a pure-flag, definitions of ghost variables and a list of contract clauses. 
Additionally, external and generated contracts contain the following elements:

**Assumed-flag:**
External and generated contracts can contain an `assumed`-flag. If this flag is set, it is assumed that the implementation of the function conforms to the provided contract without checking this. 
This can be used when a function is used for which only a declaration but no definition is available.

**Mapping clause:**
External and generated contracts further contain a `MAPPING_CLAUSE` that determines to which functions the contract will be applied. 
The mapping clause consists of a type (`ir` or `src`) and a matching string. The matching string can be a regex using the 'Modified ECMAScript regular expression grammar' used by the C++ regex library. 
- When the type is set to `src`, the contract is applied to all functions whose source-name matches the regex. Functions that to not have an available source name (e.g. because they do not have debug-information attached) are ignored. 
- When the type is set to `ir`, the contract is applied to all functions whose name on the LLVM-IR level matches the regex. 
- If the type is not specified, `ir` is assumed by default. 

**Signature clause:**
Normally, the source-level signature of the function to which a contract is applied is inferred from the function's debug information. 
However, this is not always possible when functions from external files or libraries are used.
For these cases, external contracts can contain a clause that specifies the source-level signature of the function to which they are applied (`SIG_CLAUSE`).  
This clause specifies the arguments and the return type of the function to which the contract should be applied.

### Placement of external and generated contracts
During the specification transformation process, Spectral compiles the specifications from the source language to LLVM-IR. 
For this, additional functions are generated and placed in the source-file. 
These functions are placed in the location where the specifications are defined.

The same principle applies to external and generated contracts that are defined directly in the source file. 
However, when contracts are defined in external files, the location where the generated functions should be placed must be specified manually using location markers. 

Location markers are defined using the following syntax (where `EXPR` is an expression from the source language that does not contain `;`):
```
LOC_MARKER ::= '/*@' 'wrapper location' EXPR? ';' @*/
```
The marker contains an optional matching string which is a regex that is matched against the name of the scope where a function should be inserted. 
If no matching-string is provided, the location marker is used as a 'fallback' for all cases where otherwise no matching marker can be found.

For external contracts that are defined outside of the source file, Spectral determines the innermost parent scope of all calls to the function to which the contract is applied.
For generated contracts that are defined outside of the source file, Spectral determines the direct parent scope of the function to which the contract is applied.
The generated functions for compiling the specifications are then placed at the location marker that matches the name of the determined scope (i.e. at the marker whose scope-regex matches on the name of the determined scope). 

## Ghost Functions
Ghost functions are written within specification comments using the following grammar (where `FUNC_DEF` is a function definitions from the used source language (i.e. and arbitrary string that does not contain `@*/`) and `EXPR` is an expression from the source language that does not contain `;` ):
```
GHOST_FUNC      ::= '/*@' 'ghost' 'function'
                           CONTR_CLAUSE*
                           FUNC_DEF
                      '@*/'

CONTR_CLAUSE    ::= ('requires' | 'ensures') EXPR ';'
```

The definition of a ghost function can be an arbitrary function definition. However, it must be able to be converted into a side-effect free expression by the verifier which restricts the language features that can be used.
Ghost functions can further have an optional contract consisting of requires- and ensures-clauses. 

During the specification transformation, ghost functions are inlined at the position of their definition. 

Also note, that there can only be one ghost function definition per specification comment-block. 

## Ghost Arguments
Functions can be extended with two types of ghost arguments: *Given arguments* and *Yields arguments*.
*Given arguments* behave as additional function arguments and can be used to pass additional values to a function. 
*Yields arguments* can be used to return additional values from a function. 

Both, given- and yields-arguments are part of ghost code and can therefore only be used in specifications.

**Note:** Yields-arguments can not be referenced in the requires-clauses of a contract.

### Definition
Ghost arguments are defined as part of a function's contract using the following syntax:
```
GIVEN_DEF   ::= 'given'  EXPR ';'
YIELDS_DEF  ::= 'yields' EXPR ';'
```
where `EXPR` is a string that does not contain ';' and that represents a variable-definition in the syntax of the used source language. 

E.g. in C, a contract which defines a yields-argument looks as follows:
```
/*@
yields int min;
ensures min <= a && min <= b;
ensures min == a || min == b;
@*/
int max(int a, int b) {
    [...]
}
```
where `EXPR` corresponds to `int min`.
As seen in the example, the ghost arguments can then be used like 'regular' arguments in the specifications of the function. 

**Note:** The names of the ghost arguments must be unique and may not overlap with the names of the function's regular arguments. 

**Note:** All ghost arguments that are defined in a contract must also be mentioned at least once in one of the contract's clauses. 

## Passing
When a function that defines ghost arguments is called, its given-arguments must be provided with values, and the values that are returned through yields-arguments can be assigned to other ghost variables. For this, the following syntax is used:
```
GIVEN_BINDING  ::=  /*@ 'given'  (VAR_NAME '=' EXPR)+ ';' @*/
YIELDS_BINDING ::=  /*@ 'yields' (VAR_NAME '=' VAR_NAME)+ ';' @*/
```
Where `ARG_NAME` is the name of a ghost variable (a string that does not contain '=' and ';') and `EXPR` is an expression from the source-language (i.e. a string that does not contain ';').

The specifications are matched to the first function-call in front of their definition.
An example for a call to a C function (`bar`) that is provided with ghost arguments is given below:
```
int a = bar /*@ given g = b + 2; @*/ /*@ yields d = y  @*/ (b, c);
``` 
Here `bar` is a function that defines a given-argument `g` and a yields-argument `y`.
The variables `a`, `b`, `c` are regular variables of the program and variable `d` is a ghost variable of the function in which `bar` is called.

**Note:** When a function that defines given-arguments is called, *all* given arguments must be provided with a value. For yields-arguments this restriction does not apply, i.e. values that are returned by yields-arguments may be ignored. 

**Note:** Because the annotations are matched to the first function-call before their definition, it is generally recommended to place the annotations between the name of the function and the argument-list. If e.g. the argument list contains further function-calls, the specifications might not be associated with the correct call. 

## Ghost Assignments
To assign values to ghost variables, the following syntax can be used inside of blocks of specification statements (see section *Specification Statements*):
```
GHOST_ASSIGN ::= 'ghost' 'assign' VAR_NAME '=' EXPR ';'
```
Where `VAR_NAME` is the name of a ghost variable (i.e. a string that does not contain '=' and ';') that is defined in the current scope and `EXPR` is a side-effect free expression from the source-language (i.e. a string that does not contain ';').

**Note:** It is not allowed to assign new values to given-arguments. Therefore, it is currently only allowed to assign values to yields-arguments. 

## Specification Expressions
Expressions that are used in the high-level specification constructs (`EXPR` in the examples above) use the syntax of the used source language and must generally return a boolean when used in the high-level specification constructs.

To encode specification specific constructs, each supported language provides a specification library:
- [C specification library](doc/C_Spec_Lib.md)
- [C++ specification library](doc/Cpp_Spec_Lib.md)
- [Swift specification library](doc/Swift_Spec_Lib.md)