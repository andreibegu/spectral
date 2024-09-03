// clang-format off
// Test that the MD-node for an requires-clause is wellformed
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | FileCheck-15 %s

/*@
 requires a > 0 && a < 10;
 @*/
func bar (a: Double) -> Double {
    return a + 1.0;
}

// Locate MD of bar 
// CHECK-DAG: [[barMD:![0-9]+]] = distinct !DISubprogram(name: "bar"
// CHECK-DAG: [[argMD:![0-9]+]] = !DILocalVariable(name: "a", arg: 1, scope: [[barMD]]

// locate MD of wrapper
// CHECK-DAG: [[wrapperMD:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0", linkageName: [[wFunc:"[^"]+"]]

// COM: Locate the wrapper-function
// CHECK-DAG: define hidden swiftcc i1 @[[wFunc]](double %0) {{#[0-9]+}} !dbg {{![0-9]+}} !pallas.exprWrapper

// COM: The MD-Node of the requires-clause is wellformed
// CHECK-DAG: {{![0-9]+}} = !{!"pallas.requires", [[locMd:![0-9]+]], ptr @[[wFunc]], [[argMD]]}
// CHECK-DAG: [[locMd]] = !{!"pallas.srcLoc",