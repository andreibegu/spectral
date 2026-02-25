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
// CHECK-SAME: !pallas.fcontract [[ghostContrMD:![0-9]+]]
// CHECK-DAG: [[ghostMarkerMD]] = !{!""}

// COM: Get the DIVariables of ghostMult
// CHECK-DAG: [[gMSP:![0-9]+]] = distinct !DISubprogram(name: "ghostMult",
// CHECK-DAG: [[gMA:![0-9]+]] = !DILocalVariable(name: "a", arg: 1, scope: [[gMSP]]
// CHECK-DAG: [[gMB:![0-9]+]] = !DILocalVariable(name: "b", arg: 2, scope: [[gMSP]]


// COM: Check that the contract of the ghost function is wellformed:
// CHECK-DAG: [[ghostContrMD]] = !{[[contrLocMD:![0-9]+]], i1 true, i1 false, [[emp:![0-9]+]], [[emp]], [[requiresMD:![0-9]+]]}
// CHECK-DAG: [[contrLocMD]] = !{!"pallas.srcLoc", i64 12, i64 1, i64 12, i64 24, [[fileMD:![0-9]+]]}
// CHECK-DAG: [[requiresMD]] = !{!"pallas.requires", [[contrLocMD:![0-9]+]], ptr @[[wrapper:PALLAS_SPEC_[0-9]]], [[emp]], [[emp]], [[vars:![0-9]+]]}

// COM: Get the DIVariables of the wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "[[wrapper]]
// CHECK-DAG: [[w1A:![0-9]+]] = !DILocalVariable(name: "a", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1B:![0-9]+]] = !DILocalVariable(name: "b", arg: {{[0-9]}}, scope: [[w1SP]]

// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[m1]] = !{[[gMA]], [[w1A]]}
// CHECK-DAG: [[m2]] = !{[[gMB]], [[w1B]]}