// clang-format off
// Test that assert-statements are correctly encoded as MD-nodes. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Collect variables that are available in the encoding
// CHECK: i1 @_Z13funkyFunctionff(
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[fX:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[fY:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[fTmp:![0-9]+]]

// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} @_Z13PALLAS_SPEC_0ffb
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the assert-statement is wellformed
// CHECK-DAG: [[blockMD]] = !{{{![0-9]+}}, [[assertMD:![0-9]+]]}
// CHECK-DAG: [[assertMD]] = !{!"pallas.assert", {{![0-9]+}}, ptr @_Z13PALLAS_SPEC_0ffb, [[emp:![0-9]+]], [[emp]], [[vars:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}

// COM: Check variable-mapping
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]], [[m3:![0-9]+]]}
// CHECK-DAG: [[m1]] = !{[[fX]], [[w1X:![0-9]+]]}
// CHECK-DAG: [[m2]] = !{[[fY]], [[w1Y:![0-9]+]]}
// CHECK-DAG: [[m3]] = !{[[fTmp]], [[w1Tmp:![0-9]+]]}
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-DAG: [[w1X]] = !DILocalVariable(name: "x", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1Y]] = !DILocalVariable(name: "y", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1Tmp]] = !DILocalVariable(name: "tmp", arg: {{[0-9]}}, scope: [[w1SP]]


bool funkyFunction (float x, float y) {
    bool tmp = x > y;
    /*@
    assert tmp == x > y;
    @*/
    return tmp; 
}