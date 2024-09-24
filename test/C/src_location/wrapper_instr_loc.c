// clang-format off
// Test that the locations of instructions within the generated wrapper-functions
// are translated correctly. 
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s

/*@
requires x + 1 > 0;
@*/
int bar (float x) {
    return x - 1; 
}

// COM: Match the IR and remember the MD-nodes of the debug-locations
// CHECK: define dso_local zeroext i1 @PALLAS_SPEC_0(float noundef %0)
// CHECK-SAME: !dbg [[sProgMD:![0-9]+]]

// CHECK-NEXT:  call void @llvm.dbg.value(metadata float %0, metadata [[argMD:![0-9]+]]
// CHECK-SAME: !dbg [[dbgValMD:![0-9]+]]

// CHECK-NEXT: %2 = fadd float %0, 1.000000e+00, !dbg [[addMD:![0-9]+]]
// CHECK-NEXT: %3 = fcmp ogt float %2, 0.000000e+00, !dbg [[fcmpMD:![0-9]+]]
// CHECK-NEXT: ret i1 %3
// COM: Check that the return-instruction does not have a debug-location
// CHECK-NOT: !dbg
// CHECK-NEXT: }

// COM: Check that the scope of the variable that is passed as an argument is set correctly
// CHECK-DAG: [[argMD]] = !DILocalVariable(name: "x", arg: 1, scope: [[sProgMD]]
// CHECK-SAME: line: 7

// COM: Check that the locations are set correctly:
// CHECK: [[dbgValMD]] = !DILocation(line: 0, scope: [[sProgMD]])
// CHECK: [[addMD]] = !DILocation(line: 7, column: 12, scope: [[sProgMD]])
// CHECK: [[fcmpMD]] = !DILocation(line: 7, column: 16, scope: [[sProgMD]])
