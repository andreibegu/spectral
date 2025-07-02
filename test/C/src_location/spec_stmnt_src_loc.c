// clang-format off
// Test that source-locations are correctly added to specification statements.
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s

int bar (int x) {
    /*@ 
assume x > 0;
        assert x - 1 >= 0; @*/
    return x - 1; 
}

// COM: Find MD-node of statement-block
// CHECK: !pallas.stmntBlock [[mdStmntBlock:![0-9]+]]


// COM: MD-node of the statement-block has location-info

// CHECK-DAG: [[fileMD:![0-9]+]] = !DIFile(filename: "{{.*}}spec_stmnt_src_loc.c", directory: "", checksumkind: CSK_MD5, checksum: "

// CHECK-DAG: [[mdStmntBlock]] = !{[[stmntBlockLoc:![0-9]+]], [[assumeMD:![0-9]+]], [[assertMD:![0-9]+]]}
// CHECK-DAG: [[stmntBlockLoc]] = !{!"pallas.srcLoc", i64 6, i64 5, i64 8, i64 28, [[fileMD]]}

// COM: MD-Node of the assume-statement has location-info
// CHECK-DAG: [[assumeMD]] = !{!"pallas.assume", [[assumeLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[assumeLoc]] = !{!"pallas.srcLoc", i64 7, i64 1, i64 7, i64 13, [[fileMD]]}

// COM: MD-Node of the assert-statement has location-info
// CHECK-DAG: [[assertMD]] = !{!"pallas.assert", [[assertLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[assertLoc]] = !{!"pallas.srcLoc", i64 8, i64 9, i64 8, i64 26, [[fileMD]]}