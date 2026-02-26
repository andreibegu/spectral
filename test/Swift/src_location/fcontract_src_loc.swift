// clang-format off
// Check that the location that is attached to a function-contract is correct.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

/*@
requires a > 0 && b > 0;
ensures a > 0;
@*/
func fabulousFunction(pA a: Int, pB b: Int) -> Int {
    let c = a + b + 8
    return a + c
}


// CHECK: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: MD-Nodes of the contract have location-info 
// CHECK-DAG: [[fileMD:![0-9]+]] = !DIFile(filename: "{{.*}}fcontract_src_loc.swift", directory: "", checksumkind: CSK_MD5, checksum: "

// CHECK-DAG: [[contractMD]] = !{[[contrLoc:![0-9]+]], i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[contrLoc]] = !{!"pallas.srcLoc", i64 5, i64 1, i64 8, i64 1, [[fileMD]]}

// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", [[reqLoc:![0-9]+]], 
// CHECK-DAG: [[reqLoc]] = !{!"pallas.srcLoc", i64 6, i64 1, i64 6, i64 24, [[fileMD]]}

// CHECK-DAG: [[ensMD]] = !{!"pallas.ensures", [[ensLoc:![0-9]+]], 
// CHECK-DAG: [[ensLoc]] = !{!"pallas.srcLoc", i64 7, i64 1, i64 7, i64 14, [[fileMD]]}
