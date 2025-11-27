// Test that the sequence-types are encoded correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// CHECK-DAG: %pallas.seq.float = type { float, i64, i64, i64 }
// CHECK-DAG: %pallas.seq.double = type { double, i64, i64, i64 }

/*@
 requires pallasSpec::_seqSize(pallasSpec::_seqNew<float>()) == 0;
 requires pallasSpec::_seqSize(pallasSpec::_seqNew<double>()) == 0;
 @*/
int bar (int a, float b) {
    return a + b;
}

