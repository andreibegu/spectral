// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Check that old sequence-type is no longer present
// CHECK-NOT: F8SeqFloatL_V = type <{

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] float @"pallas.seq.get float_float"(ptr noundef byval(%pallas.seq.float), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @"pallas.seq.prepend float_float"(ptr sret(%pallas.seq.float), float, ptr noundef byval(%pallas.seq.float))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @"pallas.seq.prepend float_float"(ptr sret(%pallas.seq.float) %{{[0-9]+}}, float 0.000000e+00, ptr noundef byval(%pallas.seq.float) %{{[0-9]+}}) 
// CHECK-DAG: call float @"pallas.seq.get float_float"(ptr noundef byval(%pallas.seq.float) %{{[0-9]+}}, i64 noundef 0)

/*@
declare #defSeq<Float>;
@*/

/*@
 requires SeqFloat().prepend(0.0)[0] == 0;
 @*/
func someFunctionWithoutMeaning(_ a: Int, _ b: Int) -> Int {
    return a + b
}
