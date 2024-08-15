// clang-format off
// Test that source-locations are correctly added to loop-invariants.
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | FileCheck-15 %s

int bar (int x) {
    int i = 0;
    /*@
    loop_invariant 
      i <= x + 10
      
        && x == 6;
    @*/
    while (i < 10 ) {
      i += x;
    } 
    return i; 
}

// COM: MD-node of the invariant-block has location-info
// CHECK: = !{!"pallas.loopInv", [[invBlockLoc:![0-9]+]], [[invMD:![0-9]+]]}
// CHECK-DAG: [[invBlockLoc]] = !{!"pallas.srcLoc", !"7", !"4", !"12", !"4"}

// COM: MD-Node of the invariant has location-info 
// CHECK-DAG: [[invMD]] = !{[[invLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[invLoc]] = !{!"pallas.srcLoc", !"8", !"4", !"11", !"17"}