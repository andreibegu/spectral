// clang-format off
// Test that source-locations are correctly added to function-contracts
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s

/*@
    requires x > 0; ensures x >= 0;
@*/
float bar (float x) {
    return x - 1; 
}

// COM: Contract is attached to function
// CHECK: @bar(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: MD-Nodes of the contract have location-info 

// CHECK-DAG: [[fileMD:![0-9]+]] = !DIFile(filename: "{{.*}}fcontract_src_loc.c", directory: "", checksumkind: CSK_MD5, checksum: "

// CHECK-DAG: [[contractMD]] = !{[[contrLoc:![0-9]+]], i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[contrLoc]] = !{!"pallas.srcLoc", i64 5, i64 1, i64 7, i64 1, [[fileMD]]}

// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", [[reqLoc:![0-9]+]], 
// CHECK-DAG: [[reqLoc]] = !{!"pallas.srcLoc", i64 6, i64 5, i64 6, i64 19, [[fileMD]]}

// CHECK-DAG: [[ensMD]] = !{!"pallas.ensures", [[ensLoc:![0-9]+]], 
// CHECK-DAG: [[ensLoc]] = !{!"pallas.srcLoc", i64 6, i64 21, i64 6, i64 35, [[fileMD]]}