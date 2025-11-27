// clang-format off
// Test that expressions that include the old-construct are transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration is created and has two arguments
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] void @"pallas.old sret(%struct.S) align 4 void_noundef byval(%struct.S) align 8 ptr"(ptr sret(%struct.S) align 4, ptr noundef byval(%struct.S) align 8)

// COM: Declaration has MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-function is called
// CHECK-DAG: call void @"pallas.old sret(%struct.S) align 4 void_noundef byval(%struct.S) align 8 ptr"(ptr sret(%struct.S) align 4 {{%[0-9]+}}, ptr noundef byval(%struct.S) align 8 {{%[0-9]+}})

typedef struct S {
    int a, b, c, d, e, f, g;
} SuperStruct;

/*@
 declare DEF_OLD(SuperStruct) ;
@*/

/*@
ensures _old(SuperStruct)(s).a == s.a;
@*/
void fun (SuperStruct s) {
    int c = s.a + s.b;
}