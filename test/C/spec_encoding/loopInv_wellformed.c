// clang-format off
// Test that loop-invariants are correctly encoded in metadata
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// CHECK: void @test_fun(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[v1:%[0-9]+]], metadata [[md1:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[v2:%[0-9]+]], metadata [[md2:![0-9]+]]

// COM: Check that wrapper-function is generated
// CHECK: [[wrapperFun:@PALLAS_SPEC_[0-9]+]]

// COM: Check that MD-node of invariant references correct variables
// CHECK: !{!"pallas.loopInv", {{![0-9]+}}, [[invMD:![0-9]+]]}
// CHECK: [[invMD]] = !{{{![0-9]+}}, ptr [[wrapperFun]], [[md1]], [[md2]]}

void test_fun (float p1) {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0;
    @*/
    while (cnt < 42) {
        cnt++;
    } 
}