// Test that the functions for creating new sequences and for getting the size
// of a sequence are encoded correctly.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[sizeMD:![0-9]+]] i64 @"pallas.seq.size double"(ptr noundef byval(%pallas.seq.double))
// CHECK-DAG: declare !pallas.specLib [[newMD:![0-9]+]] void @"pallas.seq.new double"(ptr sret(%pallas.seq.double))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[sizeMD]] = !{!"pallas.seq.size"}
// CHECK-DAG: [[newMD]] = !{!"pallas.seq.new"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @"pallas.seq.new double"(ptr sret(%pallas.seq.double) %{{[0-9]+}})
// CHECK-DAG: call i64 @"pallas.seq.size double"(ptr noundef byval(%pallas.seq.double) %{{[0-9]+}})

/*@
 declare DEF_SEQ(double);
 @*/

/*@
 requires _seqSize(double)(_seqNew(double)()) == 0;
 @*/
int bar (int a, float b) {
    return a + b;
}

