// Test that ensures-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Contract is attached to function
// CHECK: float @bar(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} @PALLAS_SPEC_0
// CHECK-SAME: !pallas.exprWrapper

// COM: Get the DIVariables of bar
// CHECK-DAG: [[barSP:![0-9]+]] = distinct !DISubprogram(name: "bar",
// CHECK-DAG: [[barX:![0-9]+]] = !DILocalVariable(name: "x", arg: 1, scope: [[barSP]]

// COM: Get the DIVariables of the wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-DAG: [[w1X:![0-9]+]] = !DILocalVariable(name: "x", arg: {{[0-9]}}, scope: [[w1SP]]

// COM: MD-Node of the ensures-Clause is wellformed
// CHECK-DAG: [[contractMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[cClauseMD:![0-9]+]]}
// CHECK-DAG: [[cClauseMD]] = !{!"pallas.ensures", {{![0-9]+}}, ptr @PALLAS_SPEC_0, [[emp]], [[emp]], [[vars:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]]}
// CHECK-DAG: [[m1]] = !{[[barX]], [[w1X]]}



/*@
ensures x >= 0;
@*/
float bar (float x) {
    return x; 
}