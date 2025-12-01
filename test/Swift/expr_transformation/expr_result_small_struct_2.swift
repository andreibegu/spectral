// clang-format off
// Test that the result-construct is transformed correctly when used with 
// small structs that are passed as multiple integers and returned as a 
// compound value. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] { i32, i64, i64 } @"pallas.result { i32, i64, i64 }"()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call { i32, i64, i64 } @"pallas.result { i32, i64, i64 }"()


struct AStruct {
    var i1: Int16
    var i2: Int16
    var i3: Int64
    var i4: Int64
}

/*@
ensures _result().i1 == 1;
@*/
func funcThatReturnsAStruct() -> AStruct {
    let s: AStruct = AStruct(i1: 1, i2: 2, i3: 2, i4: 4)
    return s
}
