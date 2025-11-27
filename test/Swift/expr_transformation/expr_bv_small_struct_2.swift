// clang-format off
// Test that bound variables work correctly with small structs that are
// returned as a compound-value. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// CHECK-DAG: [[sLit:@[^ ]+]] = private unnamed_addr constant [2 x i8] c"x\00"

// COM: Declarations are created and labeled with MD-nodes
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] { i64, i64 } @"pallas.boundVar { i64, i64 }"(ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}


// COM: spec-function is called
// CHECK-DAG: call { i64, i64 } @"pallas.boundVar { i64, i64 }"(ptr [[sLit]])

struct AStruct {
    public var a: Int64
    var b: Int64
}

/*@
requires _bvAStruct("x").a > 0;
@*/
func funcThatReturnsAStruct(a: Int, b: Int) -> Int {
    return a + b
}

