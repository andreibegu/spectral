// clang-format off
// Test that the locations of instructions within the generated wrapper-functions
// are translated correctly. 
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | %FileCheck %s

/*@
requires x + 1 > 0;
@*/
int bar (int x) {
    return x - 1; 
}

// COM: Match the IR and remember the MD-nodes of the debug-locations
// CHECK: define dso_local noundef zeroext i1 @_Z13PALLAS_SPEC_0i(i32 noundef %0)
// CHECK-SAME: !dbg [[sProgMD:![0-9]+]]

// CHECK-NEXT:  call void @llvm.dbg.value
// CHECK-SAME: !dbg [[dbgValMD:![0-9]+]]

// CHECK-NEXT: %2 = add nsw i32 %0, 1, !dbg [[addMD:![0-9]+]]
// CHECK-NEXT: %3 = icmp sgt i32 %2, 0, !dbg [[icmpMD:![0-9]+]]

// COM: Check that the locations are set correctly:
// CHECK: [[dbgValMD]] = !DILocation(line: 0, scope: [[sProgMD]])
// CHECK: [[addMD]] = !DILocation(line: 7, column: 12, scope: [[sProgMD]])
// CHECK: [[icmpMD]] = !DILocation(line: 7, column: 16, scope: [[sProgMD]])