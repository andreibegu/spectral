// Test that the sequence-types are encoded correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// CHECK-DAG: %pallas.seq.{{[0-9]+}} = type { float, i64, i64, i64 }
// CHECK-DAG: %pallas.seq.{{[0-9]+}} = type { double, i64, i64, i64 }

/*@
 declare DEF_SEQ(double);
 declare DEF_SEQ(float);
 @*/

/*@
 requires SEQ_SIZE(float)(SEQ_NEW(float)()) == 0;
 requires SEQ_SIZE(double)(SEQ_NEW(double)()) == 0;
 @*/
int bar (int a, float b) {
    return a + b;
}

