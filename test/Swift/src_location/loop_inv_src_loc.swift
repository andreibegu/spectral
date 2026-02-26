// clang-format off
// Check that the location that is attached to a loop-invariant is correct.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s


func foo(pA a: Int, pB b: Int) -> Int {
    var res = 0
    var j = 0
    /*@
        loop_invariant j >= 0;
    @*/
    while j < a {
        res += j
        j += b
    }

    return res
}

// COM: MD-node of the invariant-block has location-info
// CHECK-DAG: [[fileMD:![0-9]+]] = !DIFile(filename: "{{.*}}loop_inv_src_loc.swift", directory: "", checksumkind: CSK_MD5, checksum: "

// CHECK-DAG: = !{!"pallas.loopInvBlock", [[invBlockLoc:![0-9]+]], [[invMD:![0-9]+]]}
// CHECK-DAG: [[invBlockLoc]] = !{!"pallas.srcLoc", i64 9, i64 5, i64 11, i64 5, [[fileMD]]}

// COM: MD-Node of the invariant has location-info 
// CHECK-DAG: [[invMD]] = !{!"pallas.loopInv", [[invLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[invLoc]] = !{!"pallas.srcLoc", i64 10, i64 9, i64 10, i64 30, [[fileMD]]}

