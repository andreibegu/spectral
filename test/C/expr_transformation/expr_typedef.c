// Check that expressions with types that use a typedef work.
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | FileCheck-15 %s

// Check that the wrapper-function has a reference-type as its argument.
// CHECK: !{{[0-9]+}} = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-SAME: type: [[wrapperType:![0-9]+]]
// CHECK-DAG: [[wrapperType]] = !DISubroutineType(types: [[typeTuple:![0-9]+]])
// CHECK-DAG: [[typeTuple]] = !{{{![0-9]+}}, [[tdefType:![0-9]+]]}
// CHECK-DAG: [[tdefType]] = !DIDerivedType(tag: DW_TAG_typedef, name: "delightfulDouble"

typedef double delightfulDouble;

/*@
requires d == 0.1234;
@*/
void foo(delightfulDouble d){
    d += 1.2;
}

