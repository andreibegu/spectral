// clang-format off
// Test that source-locations are correctly added to loop-invariants.
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s

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
// CHECK-DAG: [[fileMD:![0-9]+]] = !DIFile(filename: "{{.*}}loop_inv_src_loc.c", directory: "", checksumkind: CSK_MD5, checksum: "

// CHECK-DAG: = !{!"pallas.loopInvBlock", [[invBlockLoc:![0-9]+]], [[invMD:![0-9]+]]}
// CHECK-DAG: [[invBlockLoc]] = !{!"pallas.srcLoc", i64 7, i64 5, i64 12, i64 5, [[fileMD]]}

// COM: MD-Node of the invariant has location-info 
// CHECK-DAG: [[invMD]] = !{!"pallas.loopInv", [[invLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[invLoc]] = !{!"pallas.srcLoc", i64 8, i64 5, i64 11, i64 18, [[fileMD]]}