// clang-format off
// Test that the locations of the wrapper-funcction's instructions are 
// transformed correctly.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// Some wierdly formatted specification to check the locations of instructions 
// when the expression covers multiple lines. 
/*@
requires x 
        + 1.0 
    > 0.0;
@*/
func bar (x: Double) -> Double {
    return x - 1;
}


// COM: Match header of the wrapper-function
// CHECK: !dbg [[sProgMD:![0-9]+]] !pallas.exprWrapper {{![0-9]+}}

// COM: Match instructions of the wrappr-function
// CHECK: call void @llvm.dbg.value
// CHECK-SAME: !dbg [[dbgValMD:![0-9]+]]

// CHECK-NEXT: %1 = fadd double %0, 1.000000e+00, !dbg [[addMD:![0-9]+]]
// CHECK-NEXT: %2 = fcmp olt double 0.000000e+00, %1, !dbg [[cmpMD:![0-9]+]]
// CHECK-NEXT: ret i1 %2

// COM: Check that the locations are set correctly:
// CHECK-DAG: [[dbgValMD]] = !DILocation(line: 0, scope: [[sProgMD]])
// CHECK-DAG: [[addMD]] = !DILocation(line: 10, column: 9, scope: [[sProgMD]])
// CHECK-DAG: [[cmpMD]] = !DILocation(line: 11, column: 5, scope: [[sProgMD]])