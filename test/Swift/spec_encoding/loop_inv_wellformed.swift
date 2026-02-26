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

// COM: Get the DIVariables of bar
// CHECK-DAG: [[barSP:![0-9]+]] = distinct !DISubprogram(name: "bar",
// CHECK-DAG: [[barBlock:![0-9]+]] = distinct !DILexicalBlock(scope: [[barSP]]
// CHECK-DAG: [[barA:![0-9]+]] = !DILocalVariable(name: "a", arg: 1, scope: [[barSP]]
// CHECK-DAG: [[barR:![0-9]+]] = !DILocalVariable(name: "r", scope: [[barBlock]]
// CHECK-DAG: [[iBlock:![0-9]+]] = distinct !DILexicalBlock(scope: [[barBlock]]
// CHECK-DAG: [[barI:![0-9]+]] = !DILocalVariable(name: "i", scope: [[iBlock]]

// COM: Locate the wrapper-functions:
// CHECK-DAG: [[w0SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0", linkageName: [[wrapper0:"[^"]+"]],
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_1", linkageName: [[wrapper1:"[^"]+"]],

// COM: Check that the invariant-block is apended to the loop-annotation
// CHECK-DAG: [[loopID]] = distinct !{[[loopID]], {{![0-9]+}}, {{![0-9]+}}, [[invBlock:![0-9]+]]}

// COM: Check that the invariant-block is correctly encoded
// CHECK-DAG: [[invBlock]] = !{!"pallas.loopInvBlock", [[invLoc:![0-9]+]], [[inv1:![0-9]+]], [[inv2:![0-9]+]]}
// CHECK-DAG: [[invLoc]] = !{!"pallas.srcLoc", 

// COM: Check that the individual invariants are encoded correctly
// CHECK-DAG: [[inv1]] = !{!"pallas.loopInv", [[loc1:![0-9]+]], ptr @[[wrapper0]], [[emp:![0-9]+]], [[emp]], [[vars:![0-9]+]]}
// CHECK-DAG: [[inv2]] = !{!"pallas.loopInv", [[loc2:![0-9]+]], ptr @[[wrapper1]], [[emp]], [[emp]], {{![0-9]+}}}

// CHECK-DAG: [[emp]] = !{}

// CHECK-DAG: [[loc1]] = !{!"pallas.srcLoc",
// CHECK-DAG: [[loc2]] = !{!"pallas.srcLoc",

// COM: Check variable mapping
// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]], [[m3:![0-9]+]]}
// CHECK-DAG: [[m1]] = !{[[barA]], [[w0A:![0-9]+]]}
// CHECK-DAG: [[m2]] = !{[[barR]], [[w0R:![0-9]+]]}
// CHECK-DAG: [[m3]] = !{[[barI]], [[w0I:![0-9]+]]}
// CHECK-DAG: [[w0A]] = !DILocalVariable(name: "a", arg: {{[0-9]}}, scope: [[w0SP]]
// CHECK-DAG: [[w0R]] = !DILocalVariable(name: "r", arg: {{[0-9]}}, scope: [[w0SP]]
// CHECK-DAG: [[w0I]] = !DILocalVariable(name: "i", arg: {{[0-9]}}, scope: [[w0SP]]