// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly
// when used with small structs that are returned as compound-types and passed
// as multiple integers. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] { i64, i16 } @pallas.seq.get.0(ptr noundef byval(%pallas.seq.0), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @pallas.seq.prepend.0(ptr sret(%pallas.seq.0), i64, i16, ptr noundef byval(%pallas.seq.0))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called with the corret attributes
// CHECK-DAG: call void @pallas.seq.prepend.0(ptr sret(%pallas.seq.0) %{{[0-9]+}}, i64 {{%[0-9]+}}, i16 {{%[0-9]+}}, ptr noundef byval(%pallas.seq.0) %{{[0-9]+}})
// CHECK-DAG: call { i64, i16 } @pallas.seq.get.0(ptr noundef byval(%pallas.seq.0) %{{[0-9]+}}, i64 noundef 0)
#include <stdint.h>

typedef struct S {
    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;
    int16_t e;
} SmallStruct;

/*@
 declare DEF_SEQ(SmallStruct);
 @*/

/*@
 requires SEQ_AT(SmallStruct)(SEQ_PREPEND(SmallStruct)(s, SEQ_NEW(SmallStruct)()), 0).a == 1;
 @*/
int bar (SmallStruct s) {
    return s.a + s.b;
}

