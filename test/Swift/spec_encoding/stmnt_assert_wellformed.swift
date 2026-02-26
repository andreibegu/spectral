// clang-format off
// Test that an assert-statement is correctly encoded.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s


func bar (a: Double) -> Double {
    let b = 1.2334
    /*@
    assert a >= 0.0;
    @*/
    return a + 1.0 + b;
}



// COM: Collect variables that are available in the encoding
// CHECK: define hidden swiftcc double @[[bar:"[^"]+"]](double %0) {{#[0-9]+}} !dbg [[barMD:![0-9]+]] {
// CHECK: call void @llvm.dbg.value(metadata double %0, metadata [[barA:![0-9]+]]
// CHECK: call void @llvm.dbg.value(metadata double 1.233400e+00, metadata [[barB:![0-9]+]]


// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define hidden swiftcc i1 @[[wrapper:"[^"]+"]](double %0, double %1) #0 !dbg {{![0-9]+}} !pallas.exprWrapper {{![0-9]+}} {

// COM: MD-Node of the assert-statement is wellformed
// CHECK-DAG: [[blockMD]] = !{{{![0-9]+}}, [[assertMD:![0-9]+]]}
// CHECK-DAG: [[assertMD]] = !{!"pallas.assert", {{![0-9]+}}, ptr @[[wrapper]], [[emp:![0-9]+]], [[emp]], [[vars:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}

// COM: Check variable missing
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]]}
// CHECK-DAG: [[m1]] = !{[[barA]], [[w1A:![0-9]+]]}
// CHECK-DAG: [[m2]] = !{[[barB]], [[w1B:![0-9]+]]}
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-DAG: [[w1A]] = !DILocalVariable(name: "a", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1B]] = !DILocalVariable(name: "b", arg: {{[0-9]}}, scope: [[w1SP]]