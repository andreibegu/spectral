// Test that bound variables are transformed correctly when used with structs.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
// COM: Old declarations not present
// CHECK-NOT: PALLAS_BOUND_VAR___

// COM: Declaration is created and labled with md-node
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] void @"pallas.boundVar sret(%struct.S) align 4 void"(ptr sret(%struct.S) align 4, ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}

// COM: Spec-function is called with sret-attribute
// CHECK-DAG call void @"pallas.boundVar sret(%struct.S) align 4 void"(ptr sret(%struct.S) align 4 {{%[0-9]+}}, ptr {{%[0-9]+}})

typedef struct S {
    int a, b, c, d, e, f, g, h;
} SqueakyStruct;

/*@
 declare DEF_BV(SqueakyStruct);
@*/

/*@
 ensures _forall(0 <= _bv(SqueakyStruct, i).a,  true);
@*/
int fun (SqueakyStruct s) {
    return s.a + s.h;
}



