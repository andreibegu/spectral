// clang-format off
// Test that an assert-statement is correctly encoded.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | FileCheck-15 %s


func bar (a: Double) -> Double {
    let b = 1.2334
    /*@
    assert a >= 0.0;
    @*/
    return a + 1.0 + b;
}



// COM: Collect variables that are available in the encoding
// CHECK: define hidden swiftcc double @[[bar:"[^"]+"]](double %0) {{#[0-9]+}} !dbg [[barMD:![0-9]+]] {
// CHECK: call void @llvm.dbg.value(metadata double %0, metadata [[aMD:![0-9]+]]
// CHECK: call void @llvm.dbg.value(metadata double 1.233400e+00, metadata [[bMD:![0-9]+]]


// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define hidden swiftcc i1 @[[wrapper:"[^"]+"]](double %0, double %1) #0 !dbg {{![0-9]+}} !pallas.exprWrapper {{![0-9]+}} {

// COM: MD-Node of the assert-statement is wellformed
// CHECK: [[blockMD]] = !{{{![0-9]+}}, [[assertMD:![0-9]+]]}
// CHECK: [[assertMD]] = !{!"pallas.assert", {{![0-9]+}}, ptr @[[wrapper]], [[aMD]], [[bMD]]}