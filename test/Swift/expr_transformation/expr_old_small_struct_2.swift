// clang-format off
// Test that the old-construct of the specification-library is correctly translated
// when used with structs where the struct is passed as multiple integers 
// and returned as a a compound-type
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] { i64, i16 } @"pallas.old { i64, i16 }_i64,i16"(i64, i16)

// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called 
// CHECK-DAG: { i64, i16 } @"pallas.old { i64, i16 }_i64,i16"(i64 {{%[0-9]+}}, i16 {{%[0-9]+}})

struct SomeStruct {
    var x: Int64
    var y: Int16
}


/*@
ensures _old(s).x == s.x;
@*/
func foo(s: SomeStruct) -> Bool{
    let a = s.x == 0
    return a
}