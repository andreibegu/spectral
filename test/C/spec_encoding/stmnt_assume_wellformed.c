// clang-format off
// Test that assume-statements are correctly encoded as MD-nodes. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Collect variables that are available in the encoding
// CHECK: float @bar(

// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@PALLAS_SPEC_[0-9]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: Get the DIVariable of bar
// CHECK-DAG: [[barSP:![0-9]+]] = distinct !DISubprogram(name: "bar",
// CHECK-DAG: [[barX:![0-9]+]] = !DILocalVariable(name: "x", arg: 1, scope: [[barSP]]

// COM: Get the DIVariables of the wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-DAG: [[w1X:![0-9]+]] = !DILocalVariable(name: "x", arg: {{[0-9]}}, scope: [[w1SP]]


// COM: MD-Node of the assert-statement is wellformed
// CHECK-DAG: [[blockMD]] = !{{{![0-9]+}}, [[assumeMD:![0-9]+]]}
// CHECK-DAG: [[assumeMD]] = !{!"pallas.assume", {{![0-9]+}}, ptr [[wrapperFunc]], [[gVars:![0-9]+]], [[gVars]], [[vars:![0-9]+]]}
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]]}
// CHECK-DAG: [[gVars]] = !{}
// CHECK-DAG: [[m1]] = !{[[barX]], [[w1X]]}


float bar (float x) {
    /*@
    assume x > 0;
    @*/
    return x + 1; 
}