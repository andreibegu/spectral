// clang-format off
// Test that assert-statements are correctly encoded as MD-nodes. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Collect variables that are available in the encoding
// CHECK: float @bar(

// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@PALLAS_SPEC_[0-9]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: Get the DIVariables of bar
// CHECK-DAG: [[barSP:![0-9]+]] = distinct !DISubprogram(name: "bar",
// CHECK-DAG: [[barX:![0-9]+]] = !DILocalVariable(name: "x", arg: 1, scope: [[barSP]]
// CHECK-DAG: [[barTmp:![0-9]+]] = !DILocalVariable(name: "tmp", scope: [[barSP]]

// COM: Get the DIVariables of the wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-DAG: [[w1X:![0-9]+]] = !DILocalVariable(name: "x", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1Tmp:![0-9]+]] = !DILocalVariable(name: "tmp", arg: {{[0-9]}}, scope: [[w1SP]]

// COM: MD-Node of the assert-statement is wellformed
// CHECK-DAG: [[blockMD]] = !{{{![0-9]+}}, [[assertMD:![0-9]+]]}
// CHECK-DAG: [[assertMD]] = !{!"pallas.assert", {{![0-9]+}}, ptr [[wrapperFunc]], [[gVars:![0-9]+]], [[gVars]], [[vars:![0-9]+]]}
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]]}
// CHECK-DAG: [[gVars]] = !{}
// CHECK-DAG: [[m1]] = !{[[barX]], [[w1X]]}
// CHECK-DAG: [[m2]] = !{[[barTmp]], [[w1Tmp]]}

float bar (float x) {
    float tmp = x + 1;
    /*@
    assert tmp == x + 1;
    @*/
    return tmp; 
}