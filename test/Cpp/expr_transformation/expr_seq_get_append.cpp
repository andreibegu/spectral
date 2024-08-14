// Test that the get- and append-functions for sequences are encoded correctly
// of a sequence are encoded correctly.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// COM: Check that old sequence-type is no longer present
// CHECK-NOT: struct.pallasSpec::seq

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] double [[get:@pallas.seq.get.[0-9]+]](ptr noundef byval(%pallas.seq.{{[0-9]+}}), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[appMD:![0-9]+]] void [[app:@pallas.seq.append.[0-9]+]](ptr sret(%pallas.seq.{{[0-9+]}}), ptr noundef byval(%pallas.seq.{{[0-9]+}}), double noundef)

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[appMD]] = !{!"pallas.seq.append"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void [[app]](ptr sret(%pallas.seq.{{[0-9]+}}) %{{[0-9]+}}, ptr noundef byval(%pallas.seq.{{[0-9]+}}) %{{[0-9]+}}, double noundef 
// CHECK-DAG: call double [[get]](ptr noundef byval(%pallas.seq.{{[0-9]+}}) %{{[0-9]+}}, i64 noundef 0)

/*@
 requires pallasSpec::seqGet(pallasSpec::seqAppend(pallasSpec::seqNew<double>(), 42.0), 0) == 42.0;
 @*/
int bar (int a, float b) {
    return a + b;
}

