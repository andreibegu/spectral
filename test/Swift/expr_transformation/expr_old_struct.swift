// clang-format off
// Test that the old-construct of the specification-library is correctly translated
// when used with structs and classes. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] void @"pallas.old noalias nocapture sret(%T13tmp_ir_source10SomeStructV) void_noalias nocapture dereferenceable(48) ptr"(ptr noalias nocapture sret([[sT:[^)]*]]), ptr noalias nocapture dereferenceable(48))

// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called 
// CHECK-DAG: call void @"pallas.old noalias nocapture sret(%T13tmp_ir_source10SomeStructV) void_noalias nocapture dereferenceable(48) ptr"(ptr noalias nocapture sret([[sT]]) {{%[0-9]+}}, ptr noalias nocapture dereferenceable(48) {{%[0-9]+}})

struct SomeStruct {
    var x: Int64
    var y: Int64
    var m1, m2, m3, m4: Int64
}


/*@
ensures _old(s).x == s.x;
@*/
func foo(s: SomeStruct) -> Bool{
    let a = s.x == 0
    return a
}