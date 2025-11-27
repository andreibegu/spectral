// clang-format off
// Test that the equality operator for sequences is encoded correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Check that declaration is present
// CHECK-DAG: declare !pallas.specLib [[eqMD:![0-9]+]] i1 @"pallas.seq.equals double"(ptr noundef byval(%pallas.seq.double), ptr noundef byval(%pallas.seq.double))

// COM: Check that the metadata-annotation is present
// CHECK-DAG: [[eqMD]] = !{!"pallas.seq.equals"}

// COM: Check that the spec-function is called
// CHECK-DAG: call i1 @"pallas.seq.equals double"(ptr noundef byval(%pallas.seq.double) %{{[0-9]+}}, ptr noundef byval(%pallas.seq.double) %{{[0-9]+}})

/*@
 requires pallasSpec::_seqEquals(pallasSpec::_seqNew<double>(), pallasSpec::_seqNew<double>());
 @*/
int bar (int a, float b) {
    return a + b;
}

