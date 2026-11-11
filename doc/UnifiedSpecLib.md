# LLVM specification-format 

## Base-encoding of specification elements
Specification elements that contain an expression use the following base-encoding which consists of a metadata-tuple containing the following operands:
- a MD-String that identifies the type of the specification element
- a reference to a MD-tuple that encodes the location in the source-file
- a pointer to the wrapper-function  that wraps the expression of the specification element 
- a reference to a MD-tuple that maps given-arguments to the arguments of the wrapper function
- a reference to a MD-tuple that maps yields-arguments map to the arguments of the wrapper function
- a reference to a MD-tuple that maps variables from the program map to the arguments of the wrapper

Depending on the type of the specification-element, additional metadata-nodes can be appended to the base-encoding

### Mapping of variables to wrapper-function arguments
The variables that are accessible in a specification are mapped to the arguments of the specification's wrapper-function using a metadata-tuple that contains a set of variable mappings. 
Each mapping is a metadata-node with two elements. 
The first element points to the DILocalVariable that corresponds to the variable that is accessible in the specification. 
The second element points to the DILocalVariable that is associated with the corresponding argument of the wrapper-function.

### Mapping of ghost variables to wrapper-function arguments
Ghost variables that are accessible in a specification are mapped to the arguments of the specification's wrapper function using a metadata-tuple that contains a set of variable mappings. 
Each variable-mapping is a metadata node with two elements where the first element points to the metadata node that defines the ghost variable, and the second node points to the DILocalVariable that is associated with the corresponding argument of the wrapper-function. 

### Example:
The following example shows the base-encoding of specification-elements on the example of an ensures-clause of a function-contract:

**Source-specifications:**
```
/*@
yields int min;
[...]
ensures min == a || min == b;  
@*/
int max(int a, int b) {
  [...]
}
```

**IR encoding:**
```
define i1 @PALLAS_SPEC_0(i32 %0, i32 %1, i32 %2) !pallas.exprWrapper !80 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !35, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 %1, metadata !38, metadata !DIExpression())
  call void @llvm.dbg.value(metadata i32 %2, metadata !27, metadata !DIExpression())
  %4 = icmp eq i32 %2, %0
  [...]
}

[...]

!17 = !{}
!21 = !{!22, !"min"}
!22 = !{!"pallas.srcLoc", i64 9, i64 1, i64 9, i64 15, !13}
!23 = !{!"pallas.ensures", !24, ptr @PALLAS_SPEC_0, !17, !25, !32}
!24 = !{!"pallas.srcLoc", i64 10, i64 1, i64 10, i64 29, !13}
!25 = !{!26}
!26 = !{!21, !27}
!27 = !DILocalVariable(name: "min", arg: 3, scope: !28, file: !13, line: 10, type: !16)
!28 = distinct !DISubprogram(name: "PALLAS_SPEC_0", [...])
!32 = !{!33, !36}
!33 = !{!34, !35}
!34 = !DILocalVariable(name: "a", arg: 1, scope: !12, file: !13, line: 12, type: !16)
!35 = !DILocalVariable(name: "a", arg: 1, scope: !28, file: !13, line: 10, type: !16)
!36 = !{!37, !38}
!37 = !DILocalVariable(name: "b", arg: 2, scope: !12, file: !13, line: 12, type: !16)
!38 = !DILocalVariable(name: "b", arg: 2, scope: !28, file: !13, line: 10, type: !16)
```
In the IR-level encoding of the specifications
- Node `!23` contains the base-encoding of the ensures-clause
- Function `@PALLAS_SPEC_0` is the wrapper-function that belongs to the expression of the ensures-clause
- Node `!17` maps the given-arguments that are defined in the contract to the arguments of `@PALLAS_SPEC_0`. In this case, this is an empty tuple because the contract does not contain any given-arguments.
- Node `!25` maps the yields-arguments that are defined in the contract to the arguments of `@PALLAS_SPEC_0`. 
  - Concretely, node `!26` maps the yields-argument that is defined by node `!21` to the argument of `@PALLAS_SPEC_0` that is associated with DILocalVariable `!27`.
- Node `!32` maps the variables from function `max` that are accessible in the ensures-clause to the arguments of `@PALLAS_SPEC_0`:
  - Node `!33` maps variable 'a' of 'max' (`!34`) to the argument of `@PALLAS_SPEC_0` that is associated with DILocalVariable `!35`
  - Node `!36` maps variable 'b' of 'max' (`!37`) to the argument of `@PALLAS_SPEC_0` that is associated with DILocalVariable `!38`


## Function contracts
Function-contracts are encoded by attaching a metadata-node to the corresponding function. 
The node is labeled as `!pallas.fcontract` and consists of a MD-tuple that consists of:
- a reference to a MD-tuple that encodes the location in the source-file
- a boolean value that specifies if the contract is marked as pure
- a `i8` value that specifies if the contract should be assumed:
  - `0`: The contract is not assumed
  - `1`: The contract should be assumed
  - `2`: The contract should be assumed transitively. I.e. functions that are only called from the assumed function should not be checked. 
- A reference to a MD-Tuple that defines a set of given-arguments
- A reference to a MD-Tuple that defines a set of yields-arguments
- followed by a list of the clauses of the contract. 

The individual clauses of the contract are encoded using the base-encoding introduced above. 
In the base-encoding, the following identifier-strings are used to specify the clause-types:
- `!"pallas.requires"` to represent a requires-clause
- `!"pallas.ensures"` to represent an ensures-clause

### Ghost argument-definitions:
The function-contract contains two lists of ghost-variable definitions (given- and yields-arguments).
Each ghost-variable definition is encoded as a metadata node with exactly two elements:
- The first element points to a metadata-node that encodes the source-location of the definition
- The second element is a MDString that contains the name of the ghost variable

The type of the ghost variable is not explicitly encoded but must be inferred from the signature of the wrapper-functions that use the ghost variable. 


### Example (C++):

**Source-contract:**
```
/*@
yields int min;
ensures min == a || min == b; 
@*/
int max(int a, int b) {
  [...]
}
```

**IR-contract:**
```
define i32 @max(i32 %0, i32 %1) !pallas.fcontract !18 {
  [...]
}

[...]

!17 = !{}
!18 = !{!19, i1 false, i1 false, !17, !20, !23}
!19 = !{!"pallas.srcLoc", i64 8, i64 1, i64 11, i64 1, !13}
!20 = !{!21}
!21 = !{!22, !"min"}
!22 = !{!"pallas.srcLoc", i64 9, i64 1, i64 9, i64 15, !13}
!23 = !{!"pallas.ensures", !24, ptr @PALLAS_SPEC_0, !17, !25, !32}
```
Here:
- Node `!18` encodes the contract
- Node `!17` encodes the definitions of given-arguments
- Node `!20` encodes the definitions of yields-arguments, concretely, node `!21` encodes the definition of 'min'

### External & Generated Contracts
Generated contracts and external contracts that are attached to a function *definition* are encoded identically to function contracts. 

External contracts that are attached to a function *declaration* are encoded similarly to regular contracts with the following differences:
- The metadata-node of the contract is labelled as `!pallas.extContract` (instead of being labelled as `!pallas.fcontract`)
- In the metadata nodes that encode the clauses of the contract, the mappings of variables to arguments of the wrapper-function are always empty. Instead, the values that should be passed to the wrapper functions are encoded implicitly. I.e. for external contracts, the signature of the corresponding wrapper functions must always be the same as the signature of the function to which the function is applied. The values passed to the wrapper function then correspond to the arguments of the function to which the contract is attached. 

### Ghost contracts
The encoding of a ghost contract (i.e. a contract that is attached to a ghost function) is identical to the encoding of a regular function contract. 

## Loop invariants
Loop-invariants are encoded similarly to function-contracts. 
The metadata-node that encodes the loop-invariant is added to the back of the [`!llvm.loop`-node](https://llvm.org/docs/LangRef.html#llvm-loop) that identifies the loop.
This assumes that all loops are in [Loop Simplify Form](https://llvm.org/docs/LoopTerminology.html#loop-simplify-form). 

The tuple that encodes the set of loop-invariants (`!128`) consists of 
- a MD-string that identifies the tuple as a set of loop invariants (`!"pallas.loopInvBlock"`)
- a reference to a MD-tuple that encodes the location in the source-file
- a list of MD-nodes that encode the individual invariants. 

The individual loop-invariants are encoded using the base-encoding introduced above using the identifier string `!"pallas.loopInv"`.


### Example (C++):

**Source-invariant:**
```
int foo (int a, int b) {
    /*@
    loop_invariant i >= 0;
    @*/
    int i = 0;
    while (a <= b) {
        a += b;
        i += 1;
    }
    return a;
}
```

**IR-invariant:**
```
define i32 @_Z3fooii(i32 %0, i32 %1) {
  [...]
  br label %6, !llvm.loop !126
  [...]
}

!126 = distinct !{!126, !118, !127, !128, !129}
!129 = !{!"pallas.loopInvBlock", !130, !132}
!130 = !{!"pallas.srcLoc", i64 2, i64 5, i64 4, i64 5, !131}
!131 = !DIFile([...])
!132 = !{!"pallas.loopInv", !133, ptr @_Z13PALLAS_SPEC_0iii, [...]}
```


## Specification-statements
Specification-statements like assume- or assert-statements are encoded as metadata-nodes and attached to the instruction before which they are executed. 

A block of statements is encoded as a MD-tuple that is labeled with `!pallas.stmntBlock`. 
This tuple contains:
- a reference to a MD-tuple that encodes the location in the source-file
- followed by references to the MD-nodes that encode the individual specification statements. 

The individual specification-statements are encoded using the base-encoding outlined above. 
In the base-encoding, the following identifier-strings are used to specify the clause-types:
- `!"pallas.assume"` to represent an assume-statement
- `!"pallas.assert"` to represent an assert-statement
- `!"pallas.fold"` to represent a fold-statement
- `!"pallas.unfold"` to represent an unfold-statement
- `!"pallas.gAssign"` to represent an assignment to a ghost variable. In this case, a reference to the MD-Node that encodes the definition of the ghost variable to which the value is assigned is appended to the base-encoding. 

### Example (C++):

**Source-statement:**
```
/*@
yields int min;
[...] 
@*/
int max(int a, int b) {
    if (a > b) {
        /*@
        assert a -b > 0;
        ghost assign min = b;
        @*/
        return a;
    } else { [...] }
}
```

**IR-statement:**
```
define i32 @max(i32 %0, i32 %1) !pallas.fcontract !18 {
  [...] 
  %10 = load i32, ptr %4, align 4, !pallas.stmntBlock !48
  [...]
}

[...]
!21 = !{!22, !"min"}
[...]
!48 = !{!49, !50, !61}
!49 = !{!"pallas.srcLoc", i64 14, i64 9, i64 17, i64 9, !13}
!50 = !{!"pallas.assert",  !51, ptr @PALLAS_SPEC_1, !17, !52, !56}
[...]
!61 = !{!"pallas.gAssign", !62, ptr @PALLAS_SPEC_2, !17, !63, !69, !21}
[...]
```

Here:
- Node `!48` encodes the block of specification-statements and is attached to the load-instruction
- Node `!50` encodes the assert-statement using the base-encoding
- Node `!61` encodes the ghost-assignment using the base-encoding. Note that node `!21` which encodes the definition of yields-argument 'min' is appended to the base-encoding. 


## Predicate definitions:
A predicate definition is represented as regular function definition that is marked by a metadata-node with id `!pallas.predDef`.
This node contains a boolean flag that specifies if the predicate is an inline predicate or not.

### Example (C):

**Source definition:**
```
/*@
predicate wPred(int *p) := _sep(p != NULL, _Perm(p, _writePerm));
@*/
```

**IR definition:**
```
define zeroext i1 @wPred(ptr noundef %0) #0 !dbg !44 !pallas.predDef !45 {
  %2 = alloca %pallas.fracT, align 8
  %3 = icmp ne ptr %0, null, !dbg !48
  %4 = call i1 @pallas.perm(ptr noundef %0, ptr noundef byval(%pallas.fracT) %2), !dbg !50
  %5 = call i1 @pallas.sepConj(i1 %3, i1 %4), !dbg !51
  ret i1 %5, !dbg !47
}

[...]

!45 = !{i1 false}
```

## Ghost code 

### Ghost functions
A ghost function is represented as a regular LLVM function that is marked with a metadata-node with id `!pallas.ghost`. 

Ghost functions can have an optional contract which is attached to the function as a metadata-node with id `!pallas.fContract`.
The encoding of the metadata-node of the ghost contract is identical to that of a regular contract.

#### Example (C)
*Source definition:*
```
/*@
ghost function
requires a > 0 && b > 0;
int ghost_mult(int a, int b) {
    return a * b;
}
@*/
```

*IR definition:*
```
define i32 @ghost_mult(i32 %0, i32 %1) !dbg !102 !pallas.ghost !62 !pallas.fContract !103 {
  [...]
}

[...]

!62 = !{!""}
```

### Ghost Bindings

**Given arguments:**
To supply values to a function's given-arguments, a block of given-bindings can be attached to a function call with the metadata-id `!pallas.givenBindings`.
The metadata node that encodes the block of given-bindings consists of: 
- A reference to a metadata node that encodes the source location of the block
- Followed by a list of metadata-nodes that encode the individual bindings.

Each binding encodes assigning an expression to a given-argument of the called function. 
A binding is encoded using the base-encoding that is outlined above with id-string `!"pallas.givenBinding"`. 
Additionally, a reference to the metadata-node that defines the ghost argument to which the expression's value is assigned is appended to the base-encoding. 

**Yields arguments:**
To assign the values that are returned through a function's yields-arguments, a block of yields-bindings can be attached to a function call with metadata-id `!pallas.yieldsBindings`.
The metadata node that encodes the block of yields-bindings consists of:
- A reference to a metadata node that encodes the source location of the block
- Followed by a list of metadata-nodes that encode the individual bindings.

Each binding encodes assigning the value of a yields-argument of the called function to a ghost-variable that is defined in the parent-function of the call-instruction. 
The encoding of a yields-binding is a metadata node with four operands:
- A MDString with value `!"pallas.yieldsBinding"`
- A reference to a metadata node that encodes the source location of the block
- A reference to the metadata node that encodes the definition of the ghost-variable to which the value is assigned. 
- A reference to the metadata node that encodes the definition of the called function's yields-arguments whose value is assigned. 

#### Example (Swift)
*Source definition:*
```
/*@
given  x: Int;
yields both_gt_x: Bool;
yields min: Int;
[...]
@*/
func get_max(_ a: Int, _ b: Int) -> Int { [...] }

/*@
yields min: Int;
yields both_gt: Bool;
[...]
@*/
func run() -> () {
    var a: Int = 1
    var b: Int = 42

    let max = get_max /*@ given x = 42; @*/ /*@ yields min = min; both_gt = both_gt_x; @*/ (a, b)
}

```

*IR definition:*
```
define i64 @"$get_max"(i64 %0, i64 %1) !pallas.fcontract !66 { 
  [...] 
}

define void @"$run"() !pallas.fcontract !184 {
  [...]
  %2 = call i64 @"get_max"(i64 1, i64 42), !pallas.givenBindings !221, !pallas.yieldsBindings !238
  [...]
}

; Definitions of ghost-args in 'get_max'
!70  = !{!71, !"x"}
!73  = !{!74, !"both_gt_x"}
!75  = !{!76, !"min"}
[...]
; Definition of the yields-args in 'run'
!187 = !{!188, !"min"}
!189 = !{!190, !"both_gt"}
[...]
; Encoding of the given-binding
!221 = !{!222, !223}
!222 = !{!"pallas.srcLoc", [...]}
!223 = !{!"pallas.givenBinding", !224, ptr @"PALLAS_SPEC_1", !61, !225, !233, !70}
[...]
; Encoding od the yields-bindings
!238 = !{!239, !240, !242}
!239 = !{!"pallas.srcLoc", [...]}
!240 = !{!"pallas.yieldsBinding", !241, !187, !75}
!241 = !{!"pallas.srcLoc", [...]}
!242 = !{!"pallas.yieldsBinding", !243, !189, !73}
!243 = !{!"pallas.srcLoc", [...]}
```


### Ghost assignments
The encoding of assignments to ghost variables is outlined in section *Specification-statements*.

## Expressions:
Expressions that are used as part of a specifications are wrapped in a boolean function. 
The variables that are accessible in the wrapped expression are passed as arguments to the wrapper function.

To identify generated wrapper-functions, an MD-node that contains an empty MD-string (`!126` in the example) is attached to the function and labeled with `!pallas.exprWrapper`.

### Example (C++):

In the example below, the expression from the requires-clause is wrapped in the function 
`@_Z13PALLAS_SPEC_0ii`. For all variables that are available in the expression (`a` and `b`), arguments are added to the wrapper-function (`%0`, `%1`).

The mapping of variables from function `foo` to the arguments of the wrapper-function is defined in the encoding of the requires-clause as outlined above in section *Base-encoding for specification elements*.

**Source-expression:**
```
/*@
requires a >= 0 && b >= 0;
@*/
int foo (int a, int b) { ... }
```

**IR-expression:**
```
define dso_local noundef i32 @_Z3fooii(i32 noundef %0, i32 noundef %1) #0 !pallas.fcontract !111 {
  [...]
}

; Generated wrapper-function
define dso_local noundef zeroext i1 @_Z13PALLAS_SPEC_0ii(i32 noundef %0, i32 noundef %1) #0 !pallas.exprWrapper !126 {
  %3 = icmp sge i32 %0, 0
  br i1 %3, label %4, label %6
4:                                                ; preds = %2
  %5 = icmp sge i32 %1, 0
  br label %6
6:                                                ; preds = %4, %2
  %7 = phi i1 [ false, %2 ], [ %5, %4 ]
  ret i1 %7, !dbg !133
}
```

## Source-locations:
Specification-constructs like contract or blocks of invariants contain information about 
where in the source-file the specification-construct occurs. This information is intended to be 
used by verification-backends to provide meaningful error-messages. 

The intuitive way to encode this information would be to use the DILocation-nodes that 
are already a part of LLVM. However this causes LLVM's verify-pass to fail which only allows DILocation-nodes 
to be used in certain places.

As a result, we use MD-tuples to encode source-location information in specification-constructs. 
A source-location-tuple consists of:
- A MD-string that identified the tuple as a source-location (`pallas.srcLoc`)
- An integer that corresponds to the start-line
- An integer that corresponds to the start-column
- An integer that corresponds to the end-line
- An integer that corresponds to the end-column
- A DIFile metadata node that corresponds to the source-file 

Line- and column-numbers both start at 1 in order to be consistent with the numbering used by LLVM.
The start-line and -column point to the position before the first parser-token of the specification-construct.
The end-line and -column point to the position before the last parser-token of the specification-construct. 

See the examples of the previous sections for reference. 

# Unified specification-library
The unified specification-library contains a set of LLVM-IR function- and type declarations that are used to encode expressions with specification-specific constructs.  

E.g. a reference to the old value of an expression (`\old(&x)`) is represented by a call to the corresponding specification-function on the IR-level (`call i32 @"pallas.old i32_i32"(i32 %0)`). 

Because frontends for different source-languages use different specification-libraries respectively, these source-language specific libraries are transformed into the unified specification-library which is the common specification-format for the backends.  

## Function Naming
In general, the names of the functions in the specification-library do not matter. The functions can by identified by using the metadata-node that is labeled with `!pallas.specLib` and that is attached to the function. 

Even though the concrete names do not matter, we use the following naming conventions for easier readability:
- The name starts with am identifier that specifies the type of the specification construct. This identifier is the same as in the `!pallas.specLib` metadata-node that is attached to the function
- This identifier is followed by a suffix that is unique to the signature of the concrete function. In general, this means that the name includes the function signature. In the following, this suffix is represented as `SFX`.

This naming scheme is used to ensure that functions are not incorrectly merged when LLVM-IR with specifications is linked together from multiple files. 



### Example: 
The function used to represent the `\old` constructs for expressions of type `i32` is named as follows:
```
declare !pallas.specLib !42 i32 @"pallas.old i32_i32"(i32)

!42 = !{!"pallas.old"}
```
Where 
- `pallas.old` is the identifier and 
- ` i32_i32` is the signature suffix

## Type-representations
When values are passed to- or returned from functions in the specification-library, 
they can be represented in different ways. This is primarily relevant for structs which are passed differently depending on their size and on the used compiler. 
One goal of our specification translation is, that it should be usable with different backends and 
that it should keep modifications to the IR-code that is generated by the compiler to a minimum.
For this reason, we generate a different version of the specification-functions for every 
way a type is represented. 

A type representation consists of a list of types. These represent how the type is passed to / returned from the specification-function. 
Moreover, a type-representation specifies a set of attributes for each of its elements. 

### Example:
When we specify the specification-function for the 'old'-construct for the following 
Swift struct (`SmallStruct`)
```
struct SmallStruct {
    var a: Int64 = 0
    var b: Int64 = 0
    var c: Double = 0.0
}
```
the compiler passes the struct to the specification-function as its individual fields
and returns the result as a aggregate-type (`{ i64, i64, double }`).
```
define hidden swiftcc { i64, i64, double } @"output._old(output.SmallStruct) -> output.SmallStruct"(noundef i64 %0, noundef i64 %1, noundef double %2) {
...
}
```
In this case, the type-representation of the passed struct would be ```noundef i64 %0, noundef i64 %1, noundef double %2```
and the type-representation of the returned struct would be ```{ i64, i64, double }```.

In the following sections, we use the notation ```<<T>>``` to represent a type-representation 
of type ```T```.

## IR-Representation of spec-lib:

### Logical Implication
Models a boolean implication (i.e. the first argument implies the second argument). 
```
declare !pallas.specLib !144 i1 @pallas.imply(i1, i1)

!144 = !{!"pallas.imply"}
```

### Short-Circuiting And
Models a short-circuiting logical and-operator. Most programming languages 
provide this operator natively. However, during compilation, these short-circuiting 
operators are converted into branches and multiple basic blocks, which can cause 
issues with the verification-backend. Hence, we provide an alternative of this 
operator that can be handled more easily by the backend. 
```
declare !pallas.specLib !144 i1 @pallas.scAnd(i1, i1)

!144 = !{!"pallas.scAnd"}
```

### Short-Circuiting Or
Models a short-circuiting logical or-operator. 
For why this operator exists, see the description of the short-circuiting 
or-operator.  
```
declare !pallas.specLib !144 i1 @pallas.scOr(i1, i1)

!144 = !{!"pallas.scOr"}
```

### Separating conjunction
Models the separating conjunction from separation-logic.
```
declare !pallas.specLib !144 i1 @pallas.sepConj(i1, i1)

!144 = !{!"pallas.sepConj"}
```

### Old
To be used in the ensures-clause. References the value of the parameter
evaluated on the heap-state before the functions execution. 

For a type `T`, specification-functions of the following shape are constructed, 
where `<<Tp>>` is the type-representation that is used to pass a value of type `T`, and 
`<<Tr>>` is the type-representation that is used to return a value of type `T`.

```
; If the result is returned as a normal return-value
declare !pallas.specLib !143 <<Tr>> @"pallas.old SFX"(<<Tp>>)

; If the result is returned in a parameter
declare !pallas.specLib !143 void @"pallas.old SFX"(<<Tr>>, <<Tp>>)

!143 = !{!"pallas.old"}
```

### Result
To be used in the ensures-clause of a function-contract. 
Returns the return-value of the function. 

For every used representation `<<T>>` of a type `T`, a separate function is generated. 

```
; If the result is returned as a regular return-value
declare !pallas.specLib !129 <<T>> @"pallas.result SFX"()

; If the result is returned in a parameter
declare !pallas.specLib !129 void @"pallas.result SFX"(<<T>>)

!129 = !{!"pallas.result"}
```

### Result reference
To be used in the ensures-clause of a function-contract. 
Returns a pointer to the return-value of the function. 
The returned pointer can be assumed to be non-null.

Can be used instead of the regular result-construct to e.g. avoid that destructor-calls are inseted by the compiler. 

```
declare !pallas.specLib !130 ptr @pallas.resultRef()

!130 = !{!"pallas.resultRef"}
```

### Bound variables
Defines a reference to a bound variable. May only be used within a quantifier. 

For every used representation `<<T>>` of a type `T`, a different function is generated. The individual bound 
variables are distinguished by the given string-constant that is passed as the argument to the call. 
The function-names are generated by incrementing counter `X`.

Names of bound variables are only unique for a given type, i.e. 
`call float @"pallas.boundVar SFX1"(%0)` refers to a different bound variable than `call double @"pallas.boundVar SFX2"(%0)` even though both refer to string constant `%0`.

```
; If the result is returned as a regular return-value
declare !pallas.specLib !163 <<T>> @"pallas.boundVar SFX"(ptr)

; If T refers to a struct-type
declare !pallas.specLib !163 void @"pallas.boundVar SFX"(<<T>>, ptr)

!163 = !{!"pallas.boundVar"}
```

### Exists
Encodes an existential quantifier. 
The first argument defines the restrictions to the range of the quantified variables while the second argument defines the quantified expression.

Bound variables of the quantifier are not explicitly declared. Instead, the bound variables are implicitly declared by the used calls to @pallas.boundVar in the expression of the first argument. 

```
declare !pallas.specLib !164 i1 @pallas.exists(i1, i1)

!164 = !{!"pallas.exists"}
```

### Forall
Encodes a universal quantifier. 
Analogous to the existential quantifier. 

```
declare !pallas.specLib !162 i1 @pallas.forall(i1, i1)

!162 = !{!"pallas.forall"}
```

### Separating forall
Encodes a separating universal quantifier. 
Analogous to the existential quantifier. 

```
declare !pallas.specLib !161 i1 @pallas.forallSep(i1, i1)

!161 = !{!"pallas.forallSep"}
```

### Fraction-type
Fractions are represented as a struct type. The values contained in the struct 
are irrelevant, and only serve the purpose of ensuring that the struct is passed
as a struct to functions and not decomposed into its individual values. 
```
%pallas.fracT = type { i64, i64, i64, i64 }
```


### FracOf
Encodes the creation of a fraction from two integer-values. 

```
declare !pallas.specLib !130 void @pallas.fracOf(ptr sret(%pallas.fracT), i32 noundef, i32 noundef)

!130 = !{!"pallas.fracOf"}
```


### Perm
Asserts an amount of permission over the given memory location. 
The first argument specifies the memory-location, and the second argument the asserted amount of permission. 

The type of the pointer that is passed as the first argument has to be determined from the IR. 

```
declare !pallas.specLib !129 i1 @pallas.perm(ptr noundef, ptr noundef byval(%pallas.fracT))

!129 = !{!"pallas.perm"}
```

### Pointer block length
Returns the length of the allocated memory block that the given pointer points into. 

```
declare !pallas.specLib !129 i64 @pallas.ptrBlockLength(ptr noundef)

!129 = !{!"pallas.ptrBlockLength"}
```

### Pointer block offset
Returns the offset of the given pointer within its allocated memory block.
```
declare !pallas.specLib !129 i64 @pallas.ptrBlockOffset(ptr noundef)

!129 = !{!"pallas.ptrBlockOffset"}
```

### Pointer length
Syntactic sugar for (ptrBlockLength(ptr) - ptrBlockOffset(ptr))

```
declare !pallas.specLib !129 i64 @pallas.ptrLength(ptr noundef)

!129 = !{!"pallas.ptrLength"}
```

### Unfolding
Temporarily unfolds a predicate while evaluating an expression.


For a type `T`, specification-functions of the following shape are constructed where `<<Tp>>` is the type-representation that is used to pass a value of type `T`, and `<<Tr>>` is the type-representation that is used to return a value of type `T`.
The function-names have a prefix of `pallas.unfolding` and contain the representations of the passed-and ret-type in their suffix to ensure correct behavior during linking.

```
; If the result is returned as a normal return-value
declare !pallas.specLib !144 <<Tr>> @"pallas.unfolding SFX"(<<Tp>>)

; If the result is returned in a parameter
declare !pallas.specLib !144 void @"pallas.unfolding SFX"(<<Tr>>, <<Tp>>)
!144 = !{!"pallas.unfolding"}
```


### Sequence-type
A sequence of content-type `T` is encoded as a struct. The first value of the 
struct corresponds to the content type. The remaining values make sure that the 
struct is not decomposed into its individual fields, when it is passed to a 
function. The type-name is generated by incrementing a counter `X`.
```
%pallas.seq.X = type { T, i64, i64, i64 }
```

### New / empty sequence
Encodes the creation of a new, empty sequence. 

For every type of sequence (`%pallas.seq.Y`), a new function is generated.
```
declare !pallas.specLib !130 void @"pallas.seq.new SFX"(ptr sret(%pallas.seq.Y))

!130 = !{!"pallas.seq.new.X"}
```

### Size of sequence
For every type of sequence (`%pallas.seq.Y`), a new function is generated.
```
declare !pallas.specLib !129 i64 @"pallas.seq.size SFX"(ptr noundef byval(%pallas.seq.Y))

!129 = !{!"pallas.seq.size.X"}
```

### Access element of sequence
Encodes accessing an element of a sequence at a given index. 

For every type of sequence (`%pallas.seq.Y` that contains elements of type `T`) 
and representation od the content type (`<<T>>), a new function is generated.

```
; If the element is returned as a regular return-value
declare !pallas.specLib !129 <<T>> @"pallas.seq.get SFX"(ptr noundef byval(%pallas.seq.Y), i64 noundef)

; If the element is returned in a parameter
declare !pallas.specLib !129 void @"pallas.seq.get SFX"(<<T>>, ptr noundef byval(%pallas.seq.Y), i64 noundef)

!129 = !{!"pallas.seq.get"}
```

### Prepend to sequence
For every type of sequence (`%pallas.seq.Y` that contains elements of type `T`)
and representation `<<T>>` of the content-type, a new function is generated.

```
declare !pallas.specLib !130 void @"pallas.seq.prepend SFX"(ptr sret(%pallas.seq.Y), <<T>>, ptr noundef byval(%pallas.seq.Y))

!130 = !{!"pallas.seq.prepend"}
```

### Equality of sequences
For every type of sequence (`%pallas.seq.Y` that contains elements of type `T`), a new function is generated.

```
declare !pallas.specLib !130 i1 @"pallas.seq.equals SFX"(ptr noundef byval(%pallas.seq.Y), ptr noundef byval(%pallas.seq.Y))

!130 = !{!"pallas.seq.equals"}
```

### Sequence-set (Update element at index)
For every type of sequence (`%pallas.seq.Y` that contains elements of type `T`)
and representation `<<T>>` of the content-type, a new function is generated.

```
declare !pallas.specLib !130 void @"pallas.seq.set SFX"(ptr sret(%pallas.seq.Y), ptr noundef byval(%pallas.seq.Y), i64, <<T>>)

!130 = !{!"pallas.seq.set"}
```

### Sequence slice
For every type of sequence (`%pallas.seq.Y` that contains elements of type `T`)
and representation `<<T>>` of the content-type, a new function is generated.

```
declare !pallas.specLib !130 void @"pallas.seq.slice SFX"(ptr sret(%pallas.seq.Y), ptr noundef byval(%pallas.seq.Y), i64, i64)

!130 = !{!"pallas.seq.slice"}
```