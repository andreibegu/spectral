// clang-format off
// Test that the result-construct is transformed correctly when used with structs.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] void @"pallas.result noalias nocapture sret(%T13tmp_ir_source7AStructV) void"(ptr noalias nocapture sret([[sT:%[^)]+]]))
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call void @"pallas.result noalias nocapture sret(%T13tmp_ir_source7AStructV) void"(ptr noalias nocapture sret([[sT]]) {{%[0-9]+}})


struct AStruct {
    var i1, i2, i3, i4, i5: Int64
}

/*@
ensures _result().i1 == 1;
@*/
func funcThatReturnsAStruct() -> AStruct {
    let s: AStruct = AStruct(i1: 1, i2: 2, i3: 3, i4: 4, i5: 5)
    return s
}
