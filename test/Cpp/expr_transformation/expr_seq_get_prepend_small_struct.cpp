// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly 
// when used with structs that are passed and returned as a single integer.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Check that old sequence-type is no longer present
// CHECK-NOT: struct.pallasSpec::seq

// COM: Check that declaration is present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] i64 @"pallas.seq.get %class.ColorfulClass_i64"(ptr noundef byval(%"pallas.seq.%class.ColorfulClass"), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @"pallas.seq.prepend %class.ColorfulClass_i64"(ptr sret(%"pallas.seq.%class.ColorfulClass"), i64, ptr noundef byval(%"pallas.seq.%class.ColorfulClass"))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @"pallas.seq.prepend %class.ColorfulClass_i64"(ptr sret(%"pallas.seq.%class.ColorfulClass") %{{[0-9]+}}, i64 %{{[0-9]+}}, ptr noundef byval(%"pallas.seq.%class.ColorfulClass") %{{[0-9]+}})
// CHECK-DAG: call i64 @"pallas.seq.get %class.ColorfulClass_i64"(ptr noundef byval(%"pallas.seq.%class.ColorfulClass") %{{[0-9]+}}, i64 noundef 0)
#include <cstdint>

class ColorfulClass {
    public:
    int32_t a;
    int32_t b;
};

/*@
 requires pallasSpec::_seqGet(pallasSpec::_seqPrepend(c, pallasSpec::_seqNew<ColorfulClass>()), 0).a == 0;
 @*/
int bar (ColorfulClass c) {
    return c.a + c.b;
}

