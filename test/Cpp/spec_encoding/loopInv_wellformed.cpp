// clang-format off
// Test that loop-invariants are correctly encoded in metadata
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// CHECK: void @_Z8test_funf(
// CHECK-SAME: [[arg:%[0-9]+]]
// CHECK: store float [[arg]], ptr [[argLoc:%[0-9]+]], align 4
// CHECK: call void @llvm.dbg.declare(metadata ptr [[argLoc]], metadata [[argMD:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[cnt:%[0-9]+]], metadata [[cntMD:![0-9]+]]

// COM: Check that wrapper-function is generated
// CHECK: @_Z13PALLAS_SPEC_0fi
// CHECK-SAME: !pallas.exprWrapper

// COM: Check that MD-node of invariant references correct variables
// CHECK: !{!"pallas.loopInv", [[invMD:![0-9]+]]}
// CHECK: [[invMD]] = !{ptr @_Z13PALLAS_SPEC_0fi, [[argMD]], [[cntMD]]}

void test_fun (float p1) {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0;
    @*/
    while (cnt < 42) {
        cnt++;
    } 
}