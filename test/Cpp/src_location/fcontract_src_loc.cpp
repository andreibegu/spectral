// clang-format off
// Test that source-locations are correctly added to function-contracts
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | %FileCheck %s

/*@
    requires x > 0;
  ensures x >= 0;
@*/
float bar (float x) {
    return x - 1; 
}

// COM: Contract is attached to function
// CHECK: @_Z3barf(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: MD-Nodes of the contract have location-info 
// CHECK: [[contractMD]] = !{[[contrLoc:![0-9]+]], i1 false, i1 false, [[reqMD:![0-9]+]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[contrLoc]] = !{!"pallas.srcLoc", i64 5, i64 1, i64 8, i64 1}

// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", [[reqLoc:![0-9]+]], 
// CHECK-DAG: [[reqLoc]] = !{!"pallas.srcLoc", i64 6, i64 5, i64 6, i64 19}

// CHECK-DAG: [[ensMD]] = !{!"pallas.ensures", [[ensLoc:![0-9]+]], 
// CHECK-DAG: [[ensLoc]] = !{!"pallas.srcLoc", i64 7, i64 3, i64 7, i64 17}