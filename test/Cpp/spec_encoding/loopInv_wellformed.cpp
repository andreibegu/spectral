// Test that loop-invariants are correctly encoded in metadata
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang Cpp | FileCheck-15 %s

// COM: Get the md-nodes of the variables in the function
// CHECK: void @test_fun(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK: @llvm.dbg.value( 
// CHECK-SAME: [[arg1]]
// CHECK-SAME: metadata [[var1MD:![0-9]+]]
// CHECK: @llvm.dbg.value(
// CHECK-SAME: 0,
// CHECK-SAME: metadata [[var2MD:![0-9]+]]

// COM: Check that wrapper-function is generated
// CHECK: [[wrapperFun:@PALLAS_SPEC_[0-9]+]]

// COM: Check that MD-node of invariant references correct variables
// CHECK: !{!"pallas.loopInv", [[invMD:![0-9]+]]}
// CHECK: [[invMD]] = !{ptr [[wrapperFun]], [[var1MD]], [[var2MD]]}

void test_fun (float p1) {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0;
    @*/
    while (cnt < 42) {
        cnt++;
    } 
}