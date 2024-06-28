// Test that loop-invariant-blocks are correctly encoded in metadata
// RUN: %Spec2IR %s -lib %CSpecLib -m | FileCheck-15 %s

// CHECK: void @test_fun(
// COM: Check that the loop-inv node is attached to llvm.loop-node
// CHECK: !llvm.loop [[loopMD:![0-9]+]]
// CHECK: [[loopMD]] = 
// CHECK-SAME: !{{{.*}} [[invBlockMD:![0-9]+]]}

// COM: Check that the loop-inv node is wellformed
// CHECK: [[invBlockMD]] = !{!"pallas.loopInv", [[inv1MD:![0-9]+]], [[inv2MD:![0-9]+]]}

// COM: Check that the nodes of the individual invariants are wellformed
// CHECK: [[inv1MD]] = !{ptr @PALLAS_SPEC_{{[0-9]+}}, {{![0-9]+}}}
// CHECK: [[inv2MD]] = !{ptr @PALLAS_SPEC_{{[0-9]+}}, {{![0-9]+}}}

void test_fun () {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0;
    loop_invariant cnt <= 42;
    @*/
    while (cnt < 42) {
        cnt++;
    } 
}