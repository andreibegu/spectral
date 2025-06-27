// clang-format off
// Test that source-locations are correctly added to loop-invariants.
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | %FileCheck %s

int bar (int x) {
    int i = 0;
    /*@
    loop_invariant i <= x + 10
      && x == x;
    @*/
    while (i < 10 ) {
      i += x;
    } 
    return i; 
}

// COM: MD-node of the invariant-block has location-info
// CHECK-DAG: [[fileMD:![0-9]+]] = !DIFile(filename: "{{.*}}loop_inv_src_loc.cpp", directory: "")

// CHECK-DAG: = !{!"pallas.loopInv", [[invBlockLoc:![0-9]+]], [[invMD:![0-9]+]]}
// CHECK-DAG: [[invBlockLoc]] = !{!"pallas.srcLoc", i64 7, i64 5, i64 10, i64 5, [[fileMD]]}

// COM: MD-Node of the invariant has location-info 
// CHECK-DAG: [[invMD]] = !{[[invLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[invLoc]] = !{!"pallas.srcLoc", i64 8, i64 5, i64 9, i64 16, [[fileMD]]}