// Check that expressions with references get transformed
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// Check that the wrapper-function has a reference-type as its argument.
// CHECK: !{{[0-9]+}} = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-SAME: type: [[wrapperType:![0-9]+]]
// CHECK-DAG: [[wrapperType]] = !DISubroutineType(types: [[typeTuple:![0-9]+]])
// CHECK-DAG: [[typeTuple]] = !{{{![0-9]+}}, [[refType:![0-9]+]]}
// CHECK-DAG: [[refType]] = !DIDerivedType(tag: DW_TAG_reference_type,

class ClassyClass {
    public:
    int i = 0;

};

/*@
ensures c.i == 2;
@*/
void foo(ClassyClass &c){
    c.i = 2;
}
