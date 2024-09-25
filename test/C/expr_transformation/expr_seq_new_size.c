// Test that the functions for creating new sequences and for getting the size
// of a sequence are encoded correctly.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[sizeMD:![0-9]+]] i64 [[size:@pallas.seq.size.[0-9]+]](ptr noundef byval(%pallas.seq.{{[0-9]+}}))
// CHECK-DAG: declare !pallas.specLib [[newMD:![0-9]+]] void [[new:@pallas.seq.new.[0-9]+]](ptr sret(%pallas.seq.{{[0-9]+}}))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[sizeMD]] = !{!"pallas.seq.size.{{[0-9]+}}"}
// CHECK-DAG: [[newMD]] = !{!"pallas.seq.new.{{[0-9]+}}"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void [[new]](ptr sret(%pallas.seq.{{[0-9]+}}) %{{[0-9]+}})
// CHECK-DAG: call i64 [[size]](ptr noundef byval(%pallas.seq.{{[0-9]+}}) %{{[0-9]+}})

/*@
 declare DEF_SEQ(double);
 @*/

/*@
 requires SEQ_SIZE(double)(SEQ_NEW(double)()) == 0;
 @*/
int bar (int a, float b) {
    return a + b;
}

