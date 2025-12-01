// clang-format off
// Test that bound variables are transformed correctly when used with small structs
// when the small struct is handled as multiple integer-values. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
// COM: Old declarations not present
// CHECK-NOT: PALLAS_BOUND_VAR___

// COM: Declaration is created and labled with md-node
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] { i64, i16 } @"pallas.boundVar { i64, i16 }"(ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}

// COM: Spec-function is called with sret-attribute
// CHECK-DAG call { i64, i16 } @"pallas.boundVar { i64, i16 }"(ptr {{%[0-9]+}})
#include <stdint.h>

typedef struct S {
    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;
    int16_t e;
} SmallStruct;

/*@
 declare DEF_BV(SmallStruct);
@*/

/*@
 ensures _forall(0 <= _bv(SmallStruct, i).a,  true);
@*/
int fun (SmallStruct s) {
    return s.a + s.b;
}



