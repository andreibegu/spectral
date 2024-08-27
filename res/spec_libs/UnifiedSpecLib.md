# LLVM specification-format 

## Function contracts
Function-contracts are encoded by attaching a metadata-node to the corresponding function. 
The node is labeled as `!pallas.fcontract` and consists of a MD-tuple that consists of:
- a reference to a MD-tuple that encodes the location in the source-file
- followed by a list of the clauses of the contract. 

The individual clauses of the contract are encoded as metadata-tuples that consist of 
- a MD-String that identifies the type of the statement ( `!"pallas.requires"` or `!"pallas.ensures"` )
- a boolean value that specifies if the contract is marked as pure
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
!111 = !{!112, i1 false, !113, !117}
!112 = !{!"pallas.srcLoc", !"1", !"1", !"4", !"1"}
!113 = !{!"pallas.requires", !114, ptr @_Z13PALLAS_SPEC_0ii, !115, !116}
!114 = !{!"pallas.srcLoc", !"2", !"1", !"2", !"26"}
!115 = !DILocalVariable(name: "a", arg: 1, scope: !106, file: !107, line: 5, type: !20)
!116 = !DILocalVariable(name: "b", arg: 2, scope: !106, file: !107, line: 5, type: !20)
!117 = !{!"pallas.ensures", !118, ptr @_Z13PALLAS_SPEC_1ii, !115, !116}
!118 = !{!"pallas.srcLoc", !"3", !"1", !"3", !"43"}
```

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
!129 = !{!"pallas.srcLoc", !"2", !"5", !"4", !"26"}
!130 = !{!131, ptr @_Z13PALLAS_SPEC_0iii, !111, !113, !115}
```

## Specification-statements
Specification-statements like assume- or assert-statements are encoded as metadata-nodes and attached to the instruction before which they are executed. 

A block of statements is encoded as a MD-tuple that is labeled with `!pallas.stmntBlock` (`!122` in the example). 
This tuple contains:
- a reference to a MD-tuple that encodes the location in the source-file
- followed by references to the MD-nodes that encode the individual specification statements. 

Individual specification-statements are encoded as MD-nodes (`!123`) that consist of
- a MD-string that identifies the type of the statement ( `!"pallas.assume"` or `!"pallas.assert"` )
- a reference to a MD-tuple that encodes the location in the source-file
- a pointer to the function that wraps the expression of the specification-statement (`@_Z13PALLAS_SPEC_0iii`)
- A list of references to the `!DILocalVariable`s that represent the variables that are available in the expression (`!111`, `!113` and `!115`). The order of the variable-references corresponds to the order in the wrapper-function's argument list. 

**Supported constructs:**
- `assert EXPR ;`
- `assume EXPR ;`

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

!111 = !DILocalVariable(name: "a", arg: 1, scope: !106, file: !107, line: 1, type: !20)
!113 = !DILocalVariable(name: "b", arg: 2, scope: !106, file: !107, line: 1, type: !20)
!115 = !DILocalVariable(name: "i", scope: !106, file: !107, line: 2, type: !20)
!120 = !DILocalVariable(name: "j", scope: !106, file: !107, line: 6, type: !20)

; Encoding of the statement-block
!122 = !{!123, !124}
!123 = !{!"pallas.srcLoc", !"3", !"5", !"5", !"5"}
!124 = !{!"pallas.assume", !125, ptr @_Z13PALLAS_SPEC_0iii, !111, !113, !115}
```

## Ghost-code
**Not supported yet**

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

!111 = !{!112, i1 false, !113}
!112 = !{!"pallas.srcLoc", !"1", !"1", !"3", !"1"}
!113 = !{!"pallas.requires", !114, ptr @_Z13PALLAS_SPEC_0ii, !115, !116}
!114 = !{!"pallas.srcLoc", !"2", !"1", !"2", !"26"}
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
- A MD-string that contains the start-line
- A MD-string that contains the start-column
- A MD-string that contains the end-line
- A MD-string that contains the end-column

Line- and coumn-numbers both start at 1 in order to be consistent with the numbering used by LLVM.
The start-line and -column point to the position before the first parser-token of the specification-construct.
The end-line and -column point to the position before the last parser-token of the specification-construct. 

See the examples of the previous sections for reference. 

# Unified specification-library
The unified specification-library conains a set of LLVM-IR function- and type declarations that are used to encode expressions with specification-specific constructs.  

E.g. a reference to the old value of a memory location (`\old(&x)`) is represented by a call to the corresponding specification-function on the IR-level (`call i32 @pallas.old.0(ptr %0)`). 

Because frontends for different source-languages use different specification-librarys respectively, these source-language specific librarys are transformed into the unified specification-library which is the common specification-format for the backends.  

## IR-Representaion of spec-lib:
- The names of the function in the specification-library should not matter. The functions can by identified by using the metadata-node that is labeled with `!pallas.specLib`.

### Imply
Models a boolean implication (i.e. the first argument implies the second argument). 
```
declare !pallas.specLib !144 i1 @pallas.imply(i1, i1)

!144 = !{!"pallas.imply"}
```

### Separating conjunction
Models the separating conjunction from separation-logic.
```
declare !pallas.specLib !144 i1 @pallas.sepConj(i1, i1)

!144 = !{!"pallas.sepConj"}
```

### Old
To be used in the ensures-clause. Returns the value at the given menory location at the beginning of the execution of the function. 

For every type `T`, a separate function is generated. 
The names of the functions are generated by incrementing a counter X.

```
; If type T does not refer to an aggregate-type
declare !pallas.specLib !143 T @pallas.old.X(ptr)

; If type T refers to an aggregate type
declare !pallas.specLib !143 void @pallas.old.X(ptr sret(%T), ptr)


!143 = !{!"pallas.old"}
```

### Result
To be used in the ensures-clause of a function-contract. 
Returns the return-value of the function. 

For every type `T`, a separate function is generated. 
The names of the functions are generated by incrementing a counter X.

```
; If T does not refer to a struct-type
declare !pallas.specLib !129 T @pallas.result.X()

; If T refers to a struct-type
declare !pallas.specLib !129 void @pallas.result.X(ptr sret(%T))

!129 = !{!"pallas.result"}
```

### Bound variables
Defines a reference to a bound variable. May only be used within a quantifier. 

For every type `T`, a different function is generated. The individual bound 
variables are distinguished by the given string-constant that is passed as the argument to the call. 
The function-names are generated by incrementing counter `X`.

Names of bound variables are only unique for a given type, i.e. 
`call float @pallas.boundVar.0(%0)` refers to a different bound variable than `call double @pallas.boundVar.1(%0)` even though both refer to string constant `%0`.

```
; If T does not refer to a struct-type
declare !pallas.specLib !163 T @pallas.boundVar.X(ptr)

; If T refers to a struct-type
declare !pallas.specLib !163 void @pallas.boundVar.X(ptr sret(%T), ptr)

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

The type of the pointer that is passed a the first argument has to be determined from the IR. 

```
declare !pallas.specLib !129 i1 @pallas.perm(ptr noundef, ptr noundef byval(%pallas.fracT))

!129 = !{!"pallas.perm"}
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
The function-names are generated by incrementing a counter `X`.
```
declare !pallas.specLib !130 void @pallas.seq.new.X(ptr sret(%pallas.seq.Y))

!130 = !{!"pallas.seq.new.X"}
```

### Size of sequence
For every type of sequence (`%pallas.seq.Y`), a new function is generated.
The function-names are generated by incrementing a counter `X`.
```
declare !pallas.specLib !129 i64 @pallas.seq.size.X(ptr noundef byval(%pallas.seq.Y))

!129 = !{!"pallas.seq.size.X"}
```

### Access element of sequence
Encodes accessing an element of a sequence at a given index. 

For every type of sequence (`%pallas.seq.Y` that contains elements of type `T`), a new function is generated.
The function-names are generated by incrementing a counter `X`.

```
; If T is a non-struct type
declare !pallas.specLib !129 T @pallas.seq.get.X(ptr noundef byval(%pallas.seq.Y), i64 noundef)

; If T is a struct-type
declare !pallas.specLib !129 void @pallas.seq.get.X(ptr sret(T), ptr noundef byval(%pallas.seq.Y), i64 noundef)

!129 = !{!"pallas.seq.get"}
```

### Prepend to sequence
For every type of sequence (`%pallas.seq.Y` that contains elements of type `T`), a new function is generated.
The function-names are generated by incrementing a counter `X`.

```
; If T is a non-struct type
declare !pallas.specLib !130 void @pallas.seq.prepend.X(ptr sret(%pallas.seq.Y), T noundef, ptr noundef byval(%pallas.seq.Y))

; If T is a struct type
declare !pallas.specLib !130 void @pallas.seq.prepend.X(ptr sret(%pallas.seq.Y), ptr noundef byval(T), ptr noundef byval(%pallas.seq.Y))

!130 = !{!"pallas.seq.prepend.X"}
```