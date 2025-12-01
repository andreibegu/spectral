// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly
// when used with large structs that are passed and returned as pointers. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] void @"pallas.seq.get %T13tmp_ir_source7AStructV_noalias nocapture sret(%T13tmp_ir_source7AStructV) void"(ptr noalias nocapture sret(%T13tmp_ir_source7AStructV), ptr noundef byval(%"pallas.seq.%T13tmp_ir_source7AStructV"), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @"pallas.seq.prepend %T13tmp_ir_source7AStructV_noalias nocapture dereferenceable(40) ptr"(ptr sret(%"pallas.seq.%T13tmp_ir_source7AStructV"), ptr noalias nocapture dereferenceable(40), ptr noundef byval(%"pallas.seq.%T13tmp_ir_source7AStructV"))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @"pallas.seq.prepend %T13tmp_ir_source7AStructV_noalias nocapture dereferenceable(40) ptr"(ptr sret(%"pallas.seq.%T13tmp_ir_source7AStructV") %{{[0-9]+}}, ptr noalias nocapture dereferenceable(40) %{{[0-9]+}}, ptr noundef byval(%"pallas.seq.%T13tmp_ir_source7AStructV") %{{[0-9]+}}) 
// CHECK-DAG: call void @"pallas.seq.get %T13tmp_ir_source7AStructV_noalias nocapture sret(%T13tmp_ir_source7AStructV) void"(ptr noalias nocapture sret(%T13tmp_ir_source7AStructV) %{{[0-9]+}}, ptr noundef byval(%"pallas.seq.%T13tmp_ir_source7AStructV") %{{[0-9]+}}, i64 noundef 0)

struct AStruct {
    var a: Int64 = 0
    var b: Int64 = 0
    var c: Double = 0.0
    var d: Double = 0.0
    var e: Int64 = 0
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
