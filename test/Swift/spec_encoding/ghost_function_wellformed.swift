// clang-format off
// Test that the MD-encoding of ghost functions is wellformed.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

/*@
ghost function
requires a > b;
func gFunc(_ a: Int, _ b: Int) -> Int {
    return a - b;
}
@*/

/*@
ensures gFunc(10, 5) > 0;
@*/
func run () -> Int {
    return 42;
}

// COM: Get the DIVariables of gFunc
// CHECK-DAG: [[gFuncSP:![0-9]+]] = distinct !DISubprogram(name: "gFunc", linkageName: "[[gFuncName:[^"]+]]"
// CHECK-DAG: [[gFuncA:![0-9]+]] = !DILocalVariable(name: "a", arg: 1, scope: [[gFuncSP]]
// CHECK-DAG: [[gFuncB:![0-9]+]] = !DILocalVariable(name: "b", arg: 2, scope: [[gFuncSP]]

// COM: Check that ghost function definition is present and marked with ghost metadata
// CHECK-DAG: define hidden swiftcc i64 @"[[gFuncName]]"(i64 %0, i64 %1) {{.*}}!pallas.ghost [[ghostMarkerMD:![0-9]+]] {{.*}}!pallas.fcontract [[ghostContrMD:![0-9]+]]
// CHECK-DAG: [[ghostMarkerMD]] = !{!""}

// COM: Check that the contract of the ghost function is wellformed
// CHECK-DAG: [[ghostContrMD]] = !{[[contrLocMD:![0-9]+]], i1 true, i1 false, [[emp:![0-9]+]], [[emp]], [[requiresMD:![0-9]+]]}
// CHECK-DAG: [[contrLocMD]] = !{!"pallas.srcLoc", i64 7, i64 1, i64 7, i64 15, {{![0-9]+}}
// CHECK-DAG: [[requiresMD]] = !{!"pallas.requires", {{![0-9]+}}, ptr @[[wFunc:"[^"]+"]], [[emp]], [[emp]], [[vars:![0-9]+]]}

// COM: Get the DIVariables of the requires-wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_{{[0-9]+}}", linkageName: [[wFunc]]
// CHECK-DAG: [[w1A:![0-9]+]] = !DILocalVariable(name: "a", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1B:![0-9]+]] = !DILocalVariable(name: "b", arg: {{[0-9]}}, scope: [[w1SP]]

// COM: Locate the wrapper-function
// CHECK-DAG: define hidden swiftcc i1 @[[wFunc]](i64 %0, i64 %1) {{#[0-9]+}} !dbg {{![0-9]+}} !pallas.exprWrapper

// COM: Check variable mapping (a and b from gFunc map to wrapper arguments)
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]]}
// CHECK-DAG: [[m1]] = !{[[gFuncA]], [[w1A]]}
// CHECK-DAG: [[m2]] = !{[[gFuncB]], [[w1B]]}
// CHECK-DAG: [[emp]] = !{}


