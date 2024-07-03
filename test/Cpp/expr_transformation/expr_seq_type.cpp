// Test that the sequence-types are encoded correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// CHECK-DAG: %pallas.seq.{{[0-9]+}} = type { float, i64, i64, i64 }
// CHECK-DAG: %pallas.seq.{{[0-9]+}} = type { double, i64, i64, i64 }

/*@
 requires pallasSpec::seqSize(pallasSpec::seqNew<float>()) == 0;
 requires pallasSpec::seqSize(pallasSpec::seqNew<double>()) == 0;
 @*/
int bar (int a, float b) {
    return a + b;
}

