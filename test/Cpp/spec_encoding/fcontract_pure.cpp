// Test that the pure-flag of a function-contract is encoded correctly.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// CHECK: double @_Z3food(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]
// CHECK: [[contractMD]] = !{{{![0-9]+}}, i1 true, 

/*@
pure;
requires x >= 0;
ensures x >= 0;
@*/
double foo (double x) {
    return x; 
}