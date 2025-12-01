// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly 
// when used with structs that are passed as multiple integers and returned as 
// a compound value. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Check that old sequence-type is no longer present
// CHECK-NOT: struct.pallasSpec::seq

// COM: Check that declaration is present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] { i64, i32 } @"pallas.seq.get %class.ColorfulClass_{ i64, i32 }"(ptr noundef byval(%"pallas.seq.%class.ColorfulClass"), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @"pallas.seq.prepend %class.ColorfulClass_i64,i32"(ptr sret(%"pallas.seq.%class.ColorfulClass"), i64, i32, ptr noundef byval(%"pallas.seq.%class.ColorfulClass"))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @"pallas.seq.prepend %class.ColorfulClass_i64,i32"(ptr sret(%"pallas.seq.%class.ColorfulClass") %{{[0-9]+}}, i64 %{{[0-9]+}}, i32 %{{[0-9]+}}, ptr noundef byval(%"pallas.seq.%class.ColorfulClass") %{{[0-9]+}})
// CHECK-DAG: call { i64, i32 } @"pallas.seq.get %class.ColorfulClass_{ i64, i32 }"(ptr noundef byval(%"pallas.seq.%class.ColorfulClass") %{{[0-9]+}}, i64 noundef 0)
#include <cstdint>

class ColorfulClass {
    public:
    int32_t r;
    int32_t g;
    int32_t b;
};

/*@
 requires pallasSpec::_seqGet(pallasSpec::_seqPrepend(c, pallasSpec::_seqNew<ColorfulClass>()), 0).r == 0;
 @*/
int bar (ColorfulClass c) {
    return c.r + c.b + c.g;
}

