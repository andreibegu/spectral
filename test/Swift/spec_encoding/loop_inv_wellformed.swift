// clang-format off
// Test that a block of loop-invariants is encoded correctly
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

func bar (a: Double) -> Double {
    var r = 0.0;
    var i = 0.0;
    
    /*@
    loop_invariant i >= 0.0;
    loop_invariant r >= 0.0;
    @*/
    while (i < 42.0) {
        r += i;
        i += 1;
    }

    return r;
}

// COM: Identify the loop-annotation
// CHECK-DAG: !llvm.loop [[loopID:![0-9]+]]	

// COM: Locate the wrapper-functions:
// CHECK-DAG: [[wrapper0MD:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0", linkageName: [[wrapper0:"[^"]+"]],
// CHECK-DAG: [[wrapper1MD:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_1", linkageName: [[wrapper1:"[^"]+"]],

// COM: Check that the invariant-block is apended to the loop-annotation
// CHECK-DAG: [[loopID]] = distinct !{[[loopID]], {{![0-9]+}}, {{![0-9]+}}, [[invBlock:![0-9]+]]}

// COM: Check that the invariant-block is correctly encoded
// CHECK-DAG: [[invBlock]] = !{!"pallas.loopInv", [[invLoc:![0-9]+]], [[inv1:![0-9]+]], [[inv2:![0-9]+]]}
// CHECK-DAG: [[invLoc]] = !{!"pallas.srcLoc", 

// COM: Check that the individual invariants are encoded correctly
// CHECK-DAG: [[inv1]] = !{[[loc1:![0-9]+]], ptr @[[wrapper0]], [[aMD:![0-9]+]], [[rMD:![0-9]+]], [[iMD:![0-9]+]]}
// CHECK-DAG: [[inv2]] = !{[[loc2:![0-9]+]], ptr @[[wrapper1]], [[aMD]], [[rMD]], [[iMD]]}

// CHECK-DAG: [[loc1]] = !{!"pallas.srcLoc",
// CHECK-DAG: [[loc2]] = !{!"pallas.srcLoc",

// CHECK-DAG: [[aMD]] = !DILocalVariable(name: "a",
// CHECK-DAG: [[rMD]] = !DILocalVariable(name: "r",
// CHECK-DAG: [[iMD]] = !DILocalVariable(name: "i",
