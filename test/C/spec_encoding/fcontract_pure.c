// Test that a contract marked as pure is correctly encoded.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// CHECK: double @foo(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]
// CHECK-DAG: [[contractMD]] = !{{{![0-9]+}}, i1 true, i1 false, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}

/*@
pure;
requires x >= 0;
ensures x >= 0;
@*/
double foo (double x) {
    return x; 
}