// clang-format off
// Test that source-locations are correctly translated in definitions 
// of ghost functions
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s

/*@
ghost function 
requires a > 0 && b > 0 && c > 0;
int ghostFunc(int a, int b, int c) {
    return a + b
            * c;
}
@*/

// COM: Find contract wrapper
// CHECK: define dso_local zeroext i1 @PALLAS_SPEC_0(i32 noundef %0, i32 noundef %1, i32 noundef %2)

// COM: Find loc-node of instr. inside of contract: 
// CHECK: %4 = icmp sgt i32 %0, 0, !dbg [[cmpLoc:![0-9]+]]
// CHECK: br i1 %4, label {{%[0-9]+}}, label {{%[0-9]+}}, !dbg [[andLoc:![0-9]+]]

// COM: Find ghost definition
// CHECK: define dso_local i32 @ghostFunc(i32 noundef %0, i32 noundef %1, i32 noundef %2)
// CHECK-SAME: !pallas.ghost {{![0-9]+}}

// COM: Find md-locations of instructions in ghost function
// CHECK: {{%[0-9]+}} = mul nsw i32 %1, %2, !dbg [[mulLoc:![0-9]+]]
// CHECK: {{%[0-9]+}} = add nsw i32 %0, %4, !dbg [[addLoc:![0-9]+]]
// CHECK: ret i32 {{%[0-9]+}}, !dbg [[retLoc:![0-9]+]]


// COM: Check that the locations are correct
// CHECK-DAG: [[wrapperFuncMD:![0-9]+]] = distinct !DISubprogram(name: "PALLAS_SPEC_0", 
// CHECK-DAG: [[cmpLoc]] = !DILocation(line: 8, column: 12, scope: [[wrapperFuncMD]])
// CHECK-DAG: [[andLoc]] = !DILocation(line: 8, column: 16, scope: [[wrapperFuncMD]])

// CHECK-DAG: [[ghostFuncMD:![0-9]+]] = distinct !DISubprogram(name: "ghostFunc", 
// CHECK-DAG: [[mulLoc]] = !DILocation(line: 11, column: 13, scope: [[ghostFuncMD]])
// CHECK-DAG: [[addLoc]] = !DILocation(line: 10, column: 14, scope: [[ghostFuncMD]])
// CHECK-DAG: [[retLoc]] = !DILocation(line: 10, column: 5, scope: [[ghostFuncMD]])
