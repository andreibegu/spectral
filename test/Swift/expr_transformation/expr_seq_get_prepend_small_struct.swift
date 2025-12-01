// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly
// when used with structs that are treated as a single int value. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] i32 @"pallas.seq.get %T13tmp_ir_source7AStructV_i32"(ptr noundef byval(%"pallas.seq.%T13tmp_ir_source7AStructV"), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @"pallas.seq.prepend %T13tmp_ir_source7AStructV_i32"(ptr sret(%"pallas.seq.%T13tmp_ir_source7AStructV"), i32, ptr noundef byval(%"pallas.seq.%T13tmp_ir_source7AStructV"))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @"pallas.seq.prepend %T13tmp_ir_source7AStructV_i32"(ptr sret(%"pallas.seq.%T13tmp_ir_source7AStructV") %{{[0-9]+}}, i32 %{{[0-9]+}}, ptr noundef byval(%"pallas.seq.%T13tmp_ir_source7AStructV") %{{[0-9]+}}) 
// CHECK-DAG: call i32 @"pallas.seq.get %T13tmp_ir_source7AStructV_i32"(ptr noundef byval(%"pallas.seq.%T13tmp_ir_source7AStructV") %{{[0-9]+}}, i64 noundef 0)

struct AStruct {
    var a: Int16 = 0
    var b: Int16 = 0
}

/*@
declare #defSeq<AStruct>;
@*/

/*@
 requires SeqAStruct().prepend(AStruct())[0].a == 0;
 @*/
func someFunctionWithoutMeaning(_ a: Int, _ b: Int) -> Int {
    let res: Int = a + b
    return res
}
