// clang-format off
// Test that bound variables work correctly with structs
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | FileCheck-15 %s

// CHECK-DAG: [[sLit:@[^ ]+]] = private unnamed_addr constant [2 x i8] c"x\00"

// COM: Declarations are created and labeled with MD-nodes
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] void @pallas.boundVar.0(ptr sret([[sTy:%[^)]+]]), ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}


// COM: spec-function is called
// CHECK-DAG: call void @pallas.boundVar.0(ptr sret([[sTy]]) {{%[0-9]+}}, ptr [[sLit]])



struct AStruct {
    public var a: Int64
    var b: Int64
    var c: Int64
    var d: Int64
    var e: Int64
}

/*@
requires _bvAStruct("x").a > 0;
@*/
func funcThatReturnsAStruct(a: Int, b: Int) -> Int {
    return a + b
}

