// clang-format off
// Test that loop-invariants are correctly encoded in metadata
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// CHECK: void @_Z8test_funf(
// CHECK-SAME: [[arg:%[0-9]+]]
// CHECK: store float [[arg]], ptr [[argLoc:%[0-9]+]], align 4
// CHECK: call void @llvm.dbg.declare(metadata ptr [[argLoc]], metadata [[fP1:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[cnt:%[0-9]+]], metadata [[fCnt:![0-9]+]]

// COM: Check that wrapper-function is generated
// CHECK: @_Z13PALLAS_SPEC_0fi
// CHECK-SAME: !pallas.exprWrapper

// COM: Check that MD-node of invariant references correct variables
// CHECK-DAG: !{!"pallas.loopInvBlock", {{![0-9]+}}, [[invMD:![0-9]+]]}
// CHECK-DAG: [[invMD]] = !{!"pallas.loopInv", {{![0-9]+}}, ptr @_Z13PALLAS_SPEC_0fi, [[emp:![0-9]+]], [[emp]], [[vars:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}

// COM: Check variable mapping
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]]}
// CHECK-DAG: [[m1]] = !{[[fP1]], [[w1P1:![0-9]+]]}
// CHECK-DAG: [[m2]] = !{[[fCnt]], [[w1Cnt:![0-9]+]]}
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0",
// CHECK-DAG: [[w1P1]] = !DILocalVariable(name: "p1", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1Cnt]] = !DILocalVariable(name: "cnt", arg: {{[0-9]}}, scope: [[w1SP]]

void test_fun (float p1) {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0;
    @*/
    while (cnt < 42) {
        cnt++;
    } 
}