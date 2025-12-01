// Test that the sequence-types are encoded correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// CHECK-DAG: %pallas.seq.float = type { float, i64, i64, i64 }
// CHECK-DAG: %pallas.seq.double = type { double, i64, i64, i64 }

/*@
 declare DEF_SEQ(double);
 declare DEF_SEQ(float);
 @*/

/*@
 requires _seqSize(float)(_seqNew(float)()) == 0;
 requires _seqSize(double)(_seqNew(double)()) == 0;
 @*/
int bar (int a, float b) {
    return a + b;
}

