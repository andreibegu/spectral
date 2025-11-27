// clang-format off
// Test that the old-construct of the specification-library is correctly translated
// when used with structs where the struct is treated as a single integer 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] i32 @"pallas.old i32_i32"(i32)

// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called 
// CHECK-DAG: i32 @"pallas.old i32_i32"(i32 {{%[0-9]+}})

struct SomeStruct {
    var x: Int16
    var y: Int16
}


/*@
ensures _old(s).x == s.x;
@*/
func foo(s: SomeStruct) -> Bool{
    let a = s.x == 0
    return a
}