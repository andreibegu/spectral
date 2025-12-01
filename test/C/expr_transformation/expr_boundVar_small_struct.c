// clang-format off
// Test that bound variables are transformed correctly when used with small structs
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
// COM: Old declarations not present
// CHECK-NOT: PALLAS_BOUND_VAR___

// COM: Declaration is created and labled with md-node
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] i64 @"pallas.boundVar i64"(ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}

// COM: Spec-function is called with sret-attribute
// CHECK-DAG call i64 @"pallas.boundVar i64"(ptr {{%[0-9]+}})
#include <stdint.h>

typedef struct S {
    float f;
    int32_t i;
} SmallStruct;

/*@
 declare DEF_BV(SmallStruct);
@*/

/*@
 ensures _forall(0 <= _bv(SmallStruct, i).f,  true);
@*/
int fun (SmallStruct s) {
    return s.f + s.i;
}



