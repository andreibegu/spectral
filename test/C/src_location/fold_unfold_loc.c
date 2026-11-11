// clang-format off
// Test that source-locations are correctly translated in fold/ unfold statements
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s
/*@
predicate pZero(int *p) := _Perm(p, _writePerm)
                            && *p == 0;
@*/

/*@
requires pZero(a);
@*/
int foo (int *a) {
    /*@
    unfold pZero(a);
    assert *a == 0;
    fold pZero(a);
    @*/
    return 42;
}

// COM: Find MD-node of statement-block
// CHECK: !pallas.stmntBlock [[mdStmntBlock:![0-9]+]]

// COM: MD-node of the statement-block has location-info
// CHECK-DAG: [[fileMD:![0-9]+]] = !DIFile(filename: "{{.*}}fold_unfold_loc.c", directory: "", checksumkind: CSK_MD5, checksum: "

// CHECK-DAG: [[mdStmntBlock]] = !{[[stmntBlockLoc:![0-9]+]], [[unfoldMD:![0-9]+]], [[assertMD:![0-9]+]], [[foldMD:![0-9]+]]}
// CHECK-DAG: [[stmntBlockLoc]] = !{!"pallas.srcLoc", i64 13, i64 5, i64 17, i64 5, [[fileMD]]}

// COM: MD-Node of the unfold-statement has location-info
// CHECK-DAG: [[unfoldMD]] = !{!"pallas.unfold", [[unfoldLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[unfoldLoc]] = !{!"pallas.srcLoc", i64 14, i64 5, i64 14, i64 20, [[fileMD]]}

// COM: MD-Node of the fold-statement has location-info
// CHECK-DAG: [[foldMD]] = !{!"pallas.fold", [[foldLoc:![0-9]+]], ptr @ 
// CHECK-DAG: [[foldLoc]] = !{!"pallas.srcLoc", i64 16, i64 5, i64 16, i64 18, [[fileMD]]}