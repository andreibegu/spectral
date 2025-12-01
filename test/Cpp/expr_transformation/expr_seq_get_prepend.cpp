// Test that the get- and prepend-functions for sequences are encoded correctly
// of a sequence are encoded correctly.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Check that old sequence-type is no longer present
// CHECK-NOT: struct.pallasSpec::seq

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] noundef double @"pallas.seq.get double_noundef double"(ptr noundef byval(%pallas.seq.double), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @"pallas.seq.prepend double_noundef double"(ptr sret(%pallas.seq.double), double noundef, ptr noundef byval(%pallas.seq.double))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @"pallas.seq.prepend double_noundef double"(ptr sret(%pallas.seq.double) %{{[0-9]+}}, double noundef 0.000000e+00, ptr noundef byval(%pallas.seq.double) %{{[0-9]+}}) 
// CHECK-DAG: call noundef double @"pallas.seq.get double_noundef double"(ptr noundef byval(%pallas.seq.double) %{{[0-9]+}}, i64 noundef 0)

/*@
 requires pallasSpec::_seqGet(pallasSpec::_seqPrepend(0.0, pallasSpec::_seqNew<double>()), 0) == 0.0;
 @*/
int bar (int a, float b) {
    return a + b;
}

