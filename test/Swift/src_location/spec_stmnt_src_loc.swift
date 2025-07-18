// clang-format off
// Check that the location that is attached to a specification-statements is correct.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

func anAmazingFunction(b1: Bool, b2: Bool) -> Bool {
    let b3 = b2 || !b1;
    /*@ assume b2 == true; @*/
    return b3 || b1;
}


// COM: Find MD-node of statement-block
// CHECK: !pallas.stmntBlock [[mdStmntBlock:![0-9]+]]

// COM: MD-node of the statement-block has location-info
// CHECK-DAG: [[fileMD:![0-9]+]] = !DIFile(filename: "{{.*}}spec_stmnt_src_loc.swift", directory: "", checksumkind: CSK_MD5, checksum: "

// CHECK-DAG: [[mdStmntBlock]] = !{[[stmntBlockLoc:![0-9]+]], [[assumeMD:![0-9]+]]}
// CHECK-DAG: [[stmntBlockLoc]] = !{!"pallas.srcLoc", i64 7, i64 5, i64 7, i64 28, [[fileMD]]}

// COM: MD-Node of the assume-statement has location-info
// CHECK-DAG: [[assumeMD]] = !{!"pallas.assume", [[assumeLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[assumeLoc]] = !{!"pallas.srcLoc", i64 7, i64 9, i64 7, i64 26, [[fileMD]]}
