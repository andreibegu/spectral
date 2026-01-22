# LLVM specification-format 

## Function contracts
Function-contracts are encoded by attaching a metadata-node to the corresponding function. 
The node is labeled as `!pallas.fcontract` and consists of a MD-tuple that consists of:
- a reference to a MD-tuple that encodes the location in the source-file
- a boolean value that specifies if the contract is marked as pure
- a boolean value that specifies if the contract should be assumed and not checked against the implementation. 
- followed by a list of the clauses of the contract. 

The individual clauses of the contract are encoded as metadata-tuples that consist of 
- a MD-String that identifies the type of the statement ( `!"pallas.requires"` or `!"pallas.ensures"` )
- a reference to a MD-tuple that encodes the location in the source-file
- a pointer to the function that wraps the expression of the clause 
- a list of references to the `!DILocalVariable`s that represent the variables that are available in the expression. The order of the variable-references corresponds to the order in the wrapper-function's argument list. 

**Supported constructs**: 
- `requires EXPR ;`
- `ensures EXPR ;`

### Example (C++):

**Source-contract:**
```
/*@
requires a >= 0 && b >= 0;
ensures pallasSpec::result<int>() == a + b;
@*/
int foo (int a, int b) {
   return a + b;
}
```

**IR-contract:**
```
define dso_local noundef i32 @_Z3fooii(i32 noundef %0, i32 noundef %1) #0 !pallas.fcontract !111 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  call void @llvm.dbg.declare(metadata ptr %3, metadata !115, metadata !DIExpression()), !dbg !119
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !116, metadata !DIExpression()), !dbg !120
  [...]
}

[...]

; Encoding of the contract
!111 = !{!112, i1 false, i1 false, !113, !117}
!112 = !{!"pallas.srcLoc", i64 1, i64 1, i64 4, i64 1, !120}
!113 = !{!"pallas.requires", !114, ptr @_Z13PALLAS_SPEC_0ii, !115, !116}
!114 = !{!"pallas.srcLoc", i64 2, i64 1, i64 2, i64 26, !120}
!115 = !DILocalVariable(name: "a", arg: 1, scope: !106, file: !107, line: 5, type: !20)
!116 = !DILocalVariable(name: "b", arg: 2, scope: !106, file: !107, line: 5, type: !20)
!117 = !{!"pallas.ensures", !118, ptr @_Z13PALLAS_SPEC_1ii, !115, !116}
!118 = !{!"pallas.srcLoc", i64 3, i64 1, i64 3, i64 43, !120}
!120 = !DIFile(filename: "/path/filename.c", directory: "")
```

### External & Generated Contracts
External and generated contracts are encoded similarly to regular contracts with the following differences:
- The metadata-node of the contract is labled as `!pallas.extContract` (instead of being labled as `!pallas.fcontract`)
- In the metdata-nodes that encode the clauses of the contract, the list of DIVariables is always empty. Instead, the values that should be passed to the wrapper functions are encoded implicitly. I.e. for external contracts, the signature of the corresponding wrapper functions must always be the same as the signature of the function to which the function is applied. The values passed to the wrapper function then correspond to the arguments of the function to which the contract is attached. 

### Ghost contracts
The encoding of a ghost contract (i.e. a contract that is attached to a ghost function) is identical to the encoding of an external contract. However the metadata node of the contract is attached to the function with ID `!pallas.ghostContract` instead. 

## Loop invariants
Loop-invariants are encoded similarly to function-contracts. 
The metadata-node that encodes the loop-invariant is added to the back of the [`!llvm.loop`-node](https://llvm.org/docs/LangRef.html#llvm-loop) that identifies the loop (`!125` in the example).
This assumes that all loops are in [Loop Simplify Form](https://llvm.org/docs/LoopTerminology.html#loop-simplify-form). 

The tuple that encodes the set of loop-invariants (`!128`) consists of 
- a MD-string that identifies the tuple as a set of loop invariants (`!"pallas.loopInv"`)
- a reference to a MD-tuple that encodes the location in the source-file
- a list of MD-nodes that encode the individual invariants. 

The encoding of the individual invariants  (`!129`) is structured similarly to the encoding of function-contract clauses, i.e. as a MD-tuple that consists of 
- a reference to a MD-tuple that encodes the location in the source-file
- a pointer to the function that wraps the expression of the invariant 
- a list of references to the `!DILocalVariable`s that represent the variables that are available in the expression. The order of the variable-references corresponds to the order in the wrapper-function's argument list. 

**Supported constructs:**
- `loop_invariant EXPR ;`

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
define dso_local noundef i32 @_Z3fooii(i32 noundef %0, i32 noundef %1) #0 !dbg !106 {
  [...]
  br label %6, !dbg !117, !llvm.loop !125
  [...]
}

[...]

!111 = !DILocalVariable(name: "a", arg: 1, scope: !106, file: !107, line: 1, type: !20)
!113 = !DILocalVariable(name: "b", arg: 2, scope: !106, file: !107, line: 1, type: !20)
!115 = !DILocalVariable(name: "i", scope: !106, file: !107, line: 5, type: !20)
!125 = distinct !{!125, !117, !126, !127, !128}
; Encoding of invariant
!128 = !{!"pallas.loopInv", !129, !130}
!129 = !{!"pallas.srcLoc", i64 2, i64 5, i64 4, i64 26, !150}
!130 = !{!131, ptr @_Z13PALLAS_SPEC_0iii, !111, !113, !115}
!150 = !DIFile(filename: "/path/filename.c", directory: "")
```

## Specification-statements
Specification-statements like assume- or assert-statements are encoded as metadata-nodes and attached to the instruction before which they are executed. 

A block of statements is encoded as a MD-tuple that is labeled with `!pallas.stmntBlock` (`!122` in the example). 
This tuple contains:
- a reference to a MD-tuple that encodes the location in the source-file
- followed by references to the MD-nodes that encode the individual specification statements. 

Individual specification-statements are encoded as MD-nodes (`!123`) that consist of
- a MD-string that identifies the type of the statement ( `!"pallas.assume"`, `!"pallas.assert"`, `!"pallas.fold"` or `!"pallas.unfold"` )
- a reference to a MD-tuple that encodes the location in the source-file
- a pointer to the function that wraps the expression of the specification-statement (`@_Z13PALLAS_SPEC_0iii`)
- A list of references to the `!DILocalVariable`s that represent the variables that are available in the expression (`!111`, `!113` and `!115`). The order of the variable-references corresponds to the order in the wrapper-function's argument list. 

**Supported constructs:**
- `assert EXPR ;`
- `assume EXPR ;`
- `fold EXPR ;`
- `unfold EXPR ;`

### Example (C++):

**Source-statement:**
```
int foo (int a, int b) {
    int i = a + b;
    /*@
    assume i >= 0;
    @*/
    int j = 1;
    return i + j;
}
```

**IR-statement:**
```
define dso_local noundef i32 @_Z3fooii(i32 noundef %0, i32 noundef %1) #0 {
  [...]
  store i32 %9, ptr %5, align 4
  call void @llvm.dbg.declare(metadata ptr %6, metadata !120, metadata !DIExpression()), !pallas.stmntBlock !122
  store i32 1, ptr %6, align 4
  %10 = load i32, ptr %5, align 4
  %11 = load i32, ptr %6, align 4
  %12 = add nsw i32 %10, %11
  ret i32 %12
}

[...]

!100 = !DIFile(filename: "/path/filename.c", directory: "")
!111 = !DILocalVariable(name: "a", arg: 1, scope: !106, file: !107, line: 1, type: !20)
!113 = !DILocalVariable(name: "b", arg: 2, scope: !106, file: !107, line: 1, type: !20)
!115 = !DILocalVariable(name: "i", scope: !106, file: !107, line: 2, type: !20)
!120 = !DILocalVariable(name: "j", scope: !106, file: !107, line: 6, type: !20)

; Encoding of the statement-block
!122 = !{!123, !124}
!123 = !{!"pallas.srcLoc", i64 3, i64 5, i64 5, i64 5, !100}
!124 = !{!"pallas.assume", !125, ptr @_Z13PALLAS_SPEC_0iii, !111, !113, !115}
```

## Predicate definitions:
A predicate definition is represented as regular function definition that is marked by a metadata-node with id `!pallas.predDef`.
This node contains a boolean flag that specifies if the predicate is an inline predicate or not.

### Example (C):

**Source definition:**
```
/*@
predicate wPred(int *p) := _sep(p != NULL, _Perm(p, _write));
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

Ghost functions can have an optional contract which is attached to the function as a metadata-node with id `!pallas.ghostContract`.
The encoding of the metadata-node of the ghost contract is identical to that of an external contract.

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
define i32 @ghost_mult(i32 %0, i32 %1) !dbg !102 !pallas.ghost !62 !pallas.ghostContract !103 {
  [...]
}

[...]

!62 = !{!""}
```

## Expressions:
Expressions that are used as part of a specifications are wrapped in a boolean function. 
The variables that are accesible in the wrapped expression are passed as arguments to the wrapper function.

To identify generated wrapper-functions, an MD-node that contains an empty MD-string (`!126` in the example) is attached to the function and labeled with `!pallas.exprWrapper`.

### Example (C++):

In the example below, the expresion from the requires-clause is wrapped in the function 
`@_Z13PALLAS_SPEC_0ii`. For all variables that are available in the expression (`a` and `b`), arguments are added to the wrapper-function (`%0`, `%1`).

The mapping of variables from function `foo` to the arguments of the wrapper-function is defined in the encoding of the requires-clause (`!112`).

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

!100 = !DIFile(filename: "/path/filename.c", directory: "")
!111 = !{!112, i1 false, !113}
!112 = !{!"pallas.srcLoc", i64 1, i64 1, i64 3, i64 1, !100}
!113 = !{!"pallas.requires", !114, ptr @_Z13PALLAS_SPEC_0ii, !115, !116}
!114 = !{!"pallas.srcLoc", i64 2, i64 1, i64 2, i64 26, !100}
!126 = !{!""}
```

## Source-locations:
Specification-constructs like contract or blocks of invariants contain information about 
where in the source-file the specification-construct occurs. This information is intended to be 
used by verification-backends to provide meaningfull error-messages. 

The intuitive way to encode this information would be to use the DILocation-nodes that 
are already a part of LLVM. However this causes LLVM's verify-pass to fail which only allows DILocation-nodes 
to be used in certain places.

As a result, we use MD-tuples to encode source-location information in specification-constructs. 
A source-location-tuple cosists of:
- A MD-string that identified the tuple as a source-location (`pallas.srcLoc`)
- An integer that corresponds to the start-line
- An integer that corresponds to the start-column
- An integer that corresponds to the end-line
- An integer that corresponds to the end-column
- A DIFile metadata node that corresponds to the source-file 

Line- and coumn-numbers both start at 1 in order to be consistent with the numbering used by LLVM.
The start-line and -column point to the position before the first parser-token of the specification-construct.
The end-line and -column point to the position before the last parser-token of the specification-construct. 

See the examples of the previous sections for reference. 

# Unified specification-library
The unified specification-library contains a set of LLVM-IR function- and type declarations that are used to encode expressions with specification-specific constructs.  

E.g. a reference to the old value of an expression (`\old(&x)`) is represented by a call to the corresponding specification-function on the IR-level (`call i32 @"pallas.old i32_i32"(i32 %0)`). 

Because frontends for different source-languages use different specification-librarys respectively, these source-language specific librarys are transformed into the unified specification-library which is the common specification-format for the backends.  

## Function Naming
In general, the names of the functions in the specification-library do not matter. The functions can by identified by using the metadata-node that is labeled with `!pallas.specLib` and that is attached to the function. 

Even though the concrete names do not matter, we use the following naming conventions for easier readability:
- The name starts with am identifier that specifies the type of the specification construct. This identifier is the same as in the `!pallas.specLib` metadata-node that is attached to the function
- This identifier is followed by a suffix that is unique to the signature of the concrete function. In general, this means that the name includes the function signature. In the following, this suffix is represented as `SFX`.

This naming scheme is used to ensure that functions are not incorreclty merged when LLVM-IR with specifications is linked together from multiple files. 



### Example: 
The function used to repsesent the `\old` constructs for expressions of type `i32` is named as follows:
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

A type representation consits of a list of types. These represent how the type is passed to / returned from the specification-function. 
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
the compiler passes the struct the the specification-function as its individual fields
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

### Short-Circuting And
Models a short-circuting logical and-operator. Most programming languages 
provide this operator natively. However, during compilation, these short-circuting 
operators are converted into branches and multiple basic blocks, which can cause 
issues with the verification-backend. Hence, we provide an alternative of this 
operator that can be handled more easily by the backend. 
```
declare !pallas.specLib !144 i1 @pallas.scAnd(i1, i1)

!144 = !{!"pallas.scAnd"}
```

### Short-Circuting Or
Models a short-circuting logical or-operator. 
For why this operator exists, see the description of the short-circuting 
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

### Exisits
Encodes an existential quantifier. 
The first argument defines the restrictions to the range of the quantified variables while the second argument defines the quantified expression.

Bound variables of the quntifier are not explicitly declared. Instead, the bound variables are implicitly declared by the used calls to @pallas.boundVar in the expression of the first argument. 

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