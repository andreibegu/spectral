// clang-format off
// Test that the result-construct is transformed correctly when used with 
// small structs that are treated as an int.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] i32 @"pallas.result i32"()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call i32 @"pallas.result i32"()


struct AStruct {
    var i1: Int16
    var i2: Int16
}

/*@
ensures _result().i1 == 1;
@*/
func funcThatReturnsAStruct() -> AStruct {
    let s: AStruct = AStruct(i1: 1, i2: 2)
    return s
}
