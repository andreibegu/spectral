// clang-format off
// Test that requires-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Contract is attached to function
// CHECK: @_Z3funff(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK-SAME: [[arg2:%[0-9]+]]
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: Match the MD-nodes of the function arguments
// CHECK: store float [[arg1]], ptr [[arg1Loc:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[arg1Loc]], metadata [[funX:![0-9]+]]
// CHECK: store float [[arg2]], ptr [[arg2Loc:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[arg2Loc]], metadata [[funY:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK-DAG: define {{.*}} @_Z13PALLAS_SPEC_0ff
// CHECK-SAME: !pallas.exprWrapper

// COM: Get the DIVariables of the wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-DAG: [[w1X:![0-9]+]] = !DILocalVariable(name: "x", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1Y:![0-9]+]] = !DILocalVariable(name: "y", arg: {{[0-9]}}, scope: [[w1SP]]

// COM: MD-Node of the requires-Clause is wellformed
// CHECK-DAG: [[contractMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[rMD:![0-9]+]]}
// CHECK-DAG: [[rMD]] = !{!"pallas.requires", {{![0-9]+}}, ptr @_Z13PALLAS_SPEC_0ff, [[emp]], [[emp]], [[vars:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]]}
// CHECK-DAG: [[m1]] = !{[[funX]], [[w1X]]}
// CHECK-DAG: [[m2]] = !{[[funY]], [[w1Y]]}


/*@
requires x >= 0;
@*/
float fun (float x, float y) {
    return x + y; 
}