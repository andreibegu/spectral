// clang-format off
// Test that loop-invariants are correctly encoded in metadata
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// CHECK: void @test_fun(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[v1:%[0-9]+]], metadata [[md1:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[v2:%[0-9]+]], metadata [[md2:![0-9]+]]

// COM: Generated wrapper is labeled
// CHECK: define {{.*}} @PALLAS_SPEC_0
// CHECK-SAME: !pallas.exprWrapper

// COM: Get the DIVariables of test_fun
// CHECK-DAG: [[testSP:![0-9]+]] = distinct !DISubprogram(name: "test_fun",
// CHECK-DAG: [[testP1:![0-9]+]] = !DILocalVariable(name: "p1", arg: 1, scope: [[testSP]]
// CHECK-DAG: [[testCnt:![0-9]+]] = !DILocalVariable(name: "cnt", scope: [[testSP]]

// COM: Get the DIVariables of the wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-DAG: [[w1P1:![0-9]+]] = !DILocalVariable(name: "p1", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1Cnt:![0-9]+]] = !DILocalVariable(name: "cnt", arg: {{[0-9]}}, scope: [[w1SP]]

// COM: Check that MD-node of invariant is wellformed
// CHECK-DAG: !{!"pallas.loopInvBlock", {{![0-9]+}}, [[invMD:![0-9]+]]}
// CHECK-DAG: [[invMD]] = !{!"pallas.loopInv", {{![0-9]+}}, ptr @PALLAS_SPEC_0, [[gVars:![0-9]+]], [[gVars]], [[vars:![0-9]+]]}
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]]}
// CHECK-DAG: [[gVars]] = !{}
// CHECK-DAG: [[m1]] = !{[[testP1]], [[w1P1]]}
// CHECK-DAG: [[m2]] = !{[[testCnt]], [[w1Cnt]]}

void test_fun (float p1) {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0;
    @*/
    while (cnt < 42) {
        cnt++;
    } 
}