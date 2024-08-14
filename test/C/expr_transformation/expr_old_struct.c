// clang-format off
// Test that expressions that include the old-construct are transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | FileCheck-15 %s

// COM: Declaration is created and has two arguments
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] void @pallas.old.0(ptr sret(%struct.S), ptr)

// COM: Declaration has MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-function is called
// CHECK-DAG: call void @pallas.old.0(ptr sret(%struct.S) {{%[0-9]+}}, ptr {{%[0-9]+}})

typedef struct S {
    int a, b, c, d, e, f, g;
} SuperStruct;

/*@
 declare DEF_OLD(SuperStruct) ;
@*/

/*@
ensures OLD(SuperStruct)(&s).a == s.a;
@*/
void fun (SuperStruct s) {
    int c = s.a + s.b;
}