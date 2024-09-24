// Test that the get- and prepend-functions for sequences are encoded correctly
// of a sequence are encoded correctly.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Check that old sequence-type is no longer present
// CHECK-NOT: struct.pallasSpec::seq

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] double @pallas.seq.get.0(ptr noundef byval(%pallas.seq.0), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @pallas.seq.prepend.0(ptr sret(%pallas.seq.0), double noundef, ptr noundef byval(%pallas.seq.0))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @pallas.seq.prepend.0(ptr sret(%pallas.seq.0) %{{[0-9]+}}, double noundef 0.000000e+00, ptr noundef byval(%pallas.seq.0) %{{[0-9]+}}) 
// CHECK-DAG: call double @pallas.seq.get.0(ptr noundef byval(%pallas.seq.0) %{{[0-9]+}}, i64 noundef 0)

/*@
 requires pallasSpec::seqGet(pallasSpec::seqPrepend(0.0, pallasSpec::seqNew<double>()), 0) == 0.0;
 @*/
int bar (int a, float b) {
    return a + b;
}

