// clang-format off
// Test that the the equalifty-operator for sequences is correctly encoded. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Check that declaration is present
// CHECK-DAG: declare !pallas.specLib [[eqMD:![0-9]+]] i1 @"pallas.seq.equals double"(ptr noundef byval(%pallas.seq.double), ptr noundef byval(%pallas.seq.double))

// COM: Check that the metadata-annotation is present
// CHECK-DAG: [[eqMD]] = !{!"pallas.seq.equals"}

// COM: Check that the spec-function is called
// CHECK-DAG: call i1 @"pallas.seq.equals double"(ptr noundef byval(%pallas.seq.double) %{{[0-9]+}}, ptr noundef byval(%pallas.seq.double) %{{[0-9]+}})

/*@
 declare DEF_SEQ(double);
 @*/

/*@
 requires _seqEq(double)(_seqNew(double)(), _seqNew(double)());
 @*/
int bar (int a, float b) {
    return a + b;
}

