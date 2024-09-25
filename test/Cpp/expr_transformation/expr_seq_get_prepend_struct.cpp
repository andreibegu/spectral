// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly 
// when used with structs.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Check that old sequence-type is no longer present
// CHECK-NOT: struct.pallasSpec::seq

// COM: Check that declaration is present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] void @pallas.seq.get.0(ptr sret(%class.ColorfulClass), ptr noundef byval(%pallas.seq.0), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @pallas.seq.prepend.0(ptr sret(%pallas.seq.0), ptr noundef byval(%class.ColorfulClass), ptr noundef byval(%pallas.seq.0))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @pallas.seq.prepend.0(ptr sret(%pallas.seq.0) %{{[0-9]+}}, ptr noundef byval(%class.ColorfulClass) {{%[0-9]+}}, ptr noundef byval(%pallas.seq.0) %{{[0-9]+}})
// CHECK-DAG: call void @pallas.seq.get.0(ptr sret(%class.ColorfulClass) %{{[0-9]+}}, ptr noundef byval(%pallas.seq.0) %{{[0-9]+}}, i64 noundef 0)

class ColorfulClass {
    public:
    int r, g, b;
    double d1, d2, d3;
};

/*@
 requires pallasSpec::seqGet(pallasSpec::seqPrepend(c, pallasSpec::seqNew<ColorfulClass>()), 0).r == 0;
 @*/
int bar (ColorfulClass c) {
    return c.r + c.g + c.g;
}

