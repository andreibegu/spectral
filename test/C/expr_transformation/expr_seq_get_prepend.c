// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly
// of a sequence are encoded correctly.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] double [[get:@pallas.seq.get.[0-9]+]](ptr noundef byval(%pallas.seq.0), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void [[pre:@pallas.seq.prepend.[0-9]+]](ptr sret(%pallas.seq.0), double noundef, ptr noundef byval(%pallas.seq.{{[0-9]+}}))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void [[pre]](ptr sret(%pallas.seq.0) %{{[0-9]+}}, double noundef 0.000000e+00, ptr noundef byval(%pallas.seq.0) %{{[0-9]+}})
// CHECK-DAG: call double [[get]](ptr noundef byval(%pallas.seq.0) %{{[0-9]+}}, i64 noundef 0)

/*@
 declare DEF_SEQ(double);
 @*/

/*@
 requires SEQ_AT(double)(SEQ_PREPEND(double)(0, SEQ_NEW(double)()), 0) == 42;
 @*/
int bar (int a, float b) {
    return a + b;
}

