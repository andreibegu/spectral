// Test that loop-invariant-blocks are correctly encoded in metadata
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: CHECK: i32 {{@[^(]+}}(
// COM: Check that the loop-inv node is attached to llvm.loop-node
// CHECK-COUNT-1: !llvm.loop [[loopMD:![0-9]+]]
// CHECK: [[loopMD]] = 
// CHECK-SAME: !{{{.*}} [[invBlockMD:![0-9]+]]}

// COM: Check that the loop-inv node is wellformed
// CHECK-DAG: [[invBlockMD]] = !{!"pallas.loopInvBlock", {{![0-9]+}}, [[inv1MD:![0-9]+]], [[inv2MD:![0-9]+]]}

// COM: Check that the nodes of the individual invariants are wellformed
// CHECK-DAG: [[inv1MD]] = !{!"pallas.loopInv", {{![0-9]+}}, ptr {{@.+}}, [[emp:![0-9]+]], [[emp]], {{![0-9]+}}}
// CHECK-DAG: [[inv2MD]] = !{!"pallas.loopInv", {{![0-9]+}}, ptr {{@.+}}, [[emp]], [[emp]], {{![0-9]+}}}

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