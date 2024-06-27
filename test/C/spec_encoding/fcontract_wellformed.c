// Test that function-contract are correctly encoded in metadata
// RUN: rm -rf %t && split-file-15 %s %t && cd %t

//--- encode_contract.c
// Test that MD-Node for contract is attached to function and refers 
// to tuple of two MD-Nodes
// RUN: %Spec2IR encode_contract.c -lib %CSpecLib -m | FileCheck-15 %s

// CHECK: i32 @foo(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]
// CHECK: [[contractMD]] = !{[[reqMD:![0-9]+]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[reqMD]] = !{!"pallas.requires",
// CHECK-DAG: [[ensMD]] = !{!"pallas.ensures",

/*@
requires x >= 0;
ensures x >= 0;
@*/
int foo (int x) {
    return x; 
}