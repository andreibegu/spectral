// clang-format off
// Test that the MD-node for an requires-clause is wellformed
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

/*@
 requires a > 0 && a < 10;
 @*/
func bar (a: Double) -> Double {
    return a + 1.0;
}

// Locate MD of bar 
// CHECK-DAG: [[barSP:![0-9]+]] = distinct !DISubprogram(name: "bar"
// CHECK-DAG: [[barA:![0-9]+]] = !DILocalVariable(name: "a", arg: 1, scope: [[barSP]]

// locate MD of wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0", linkageName: [[wFunc:"[^"]+"]]

// COM: Locate the wrapper-function
// CHECK-DAG: define hidden swiftcc i1 @[[wFunc]](double %0) {{#[0-9]+}} !dbg {{![0-9]+}} !pallas.exprWrapper

// COM: The MD-Node of the requires-clause is wellformed
// CHECK-DAG: {{![0-9]+}} = !{!"pallas.requires", [[locMd:![0-9]+]], ptr @[[wFunc]], [[emp:![0-9]+]], [[emp]], [[vars:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[locMd]] = !{!"pallas.srcLoc",

// COM: Check variable mapping
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]]}
// CHECK-DAG: [[m1]] = !{[[barA]], [[w1A:![0-9]+]]}
// CHECK-DAG: [[w1A]] = !DILocalVariable(name: "a", arg: {{[0-9]}}, scope: [[w1SP]]