// clang-format off
// Test that the locations of instructions within the generated wrapper-functions
// are translated correctly. 
// Test where the wrapper contains multiple basic blocks and branching.
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s

/*@
requires  
    x > y && 
    y + 1 > 1; 
@*/
float bar (float x, float y) {
    return x + y - 1; 
}


// COM: Match the IR and remember the MD-nodes of the debug-locations
// CHECK: define dso_local zeroext i1 @PALLAS_SPEC_0
// CHECK-SAME: !dbg [[sProgMD:![0-9]+]]

// CHECK-NEXT: call void @llvm.dbg.value
// CHECK-SAME: !dbg [[dbgValMD:![0-9]+]]

// CHECK-NEXT: call void @llvm.dbg.value
// CHECK-SAME: !dbg [[dbgValMD]]

// CHECK-NEXT: %3 = fcmp ogt float %0, %1, !dbg [[cmpMD:![0-9]+]]
// CHECK-NEXT: br i1 %3, label %4, label %7, !dbg [[brMD:![0-9]+]]
// CHECK: 4:
// CHECK-NEXT: %5 = fadd float %1, 1.000000e+00, !dbg [[addMD:![0-9]+]]
// CHECK-NEXT: %6 = fcmp ogt float %5, 1.000000e+00, !dbg [[cmp2MD:![0-9]+]]
// CHECK: ret i1 %8
// CHECK-NOT: !dbg
// CHECK: }

// COM: Check that the locations are set correctly:
// CHECK: [[dbgValMD]] = !DILocation(line: 0, scope: [[sProgMD]])
// CHECK: [[cmpMD]] = !DILocation(line: 9, column: 7, scope: [[sProgMD]])
// CHECK: [[brMD]] = !DILocation(line: 9, column: 11, scope: [[sProgMD]])
// CHECK: [[addMD]] = !DILocation(line: 10, column: 7, scope: [[sProgMD]])
// CHECK: [[cmp2MD]] = !DILocation(line: 10, column: 11, scope: [[sProgMD]])