// clang-format off
// Test that source-locations are correctly translated in definitions 
// of ghost functions
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s

/*@
declare DEF_RESULT(int);
@*/

/*@
ghost function 
requires a > 0 && b > 0;
int ghostMult(int a, int b) {
    return a * b;
}
@*/

/*@
requires a > 0 && b > 0;
ensures _result(int) == ghostMult(a, b);
@*/
int myMult(int a, int b) {
    int res = 0;
    for (int i = 0; i < b; ++i) {
        res += a;
    }
    return res;
}

// COM: Check that Definition of ghost function is present, marked and has contract
// CHECK: define dso_local i32 @ghostMult(i32 noundef %0, i32 noundef %1) 
// CHECK-SAME: !pallas.ghost [[ghostMarkerMD:![0-9]+]] 
// CHECK-SAME: !pallas.ghostContract [[ghostContrMD:![0-9]+]]

// COM: Check that the ghost-marker marks the ghost function as side-effect free
// CHECK-DAG: [[ghostMarkerMD]] = !{!""}

// COM: Check that the contract of the ghost function is wellformed:
// CHECK: [[ghostContrMD]] = !{[[contrLocMD:![0-9]+]], i1 true, i1 false, [[requiresMD:![0-9]+]]}
// CHECK: [[contrLocMD]] = !{!"pallas.srcLoc", i64 12, i64 1, i64 12, i64 24, [[fileMD:![0-9]+]]}
// CHECK: [[requiresMD]] = !{!"pallas.requires", [[contrLocMD:![0-9]+]], ptr [[wrapper:@PALLAS_SPEC_[0-9]]]}