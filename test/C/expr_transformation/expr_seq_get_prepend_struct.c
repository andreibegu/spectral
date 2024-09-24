// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly
// when used with struct-types. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] void @pallas.seq.get.0(ptr sret(%struct.S), ptr noundef byval(%pallas.seq.0), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @pallas.seq.prepend.0(ptr sret(%pallas.seq.0), ptr noundef byval(%struct.S), ptr noundef byval(%pallas.seq.0))


// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called with the corret attributes
// CHECK-DAG: call void @pallas.seq.prepend.0(ptr sret(%pallas.seq.0) %{{[0-9]+}}, ptr noundef byval(%struct.S) {{%[0-9]+}}, ptr noundef byval(%pallas.seq.0) %{{[0-9]+}})
// CHECK-DAG: call void @pallas.seq.get.0(ptr sret(%struct.S) %{{[0-9]+}}, ptr noundef byval(%pallas.seq.0) %{{[0-9]+}}, i64 noundef 0)

typedef struct S {
    int a, b, c, d, e, f, g;
} SuperStruct;

/*@
 declare DEF_SEQ(SuperStruct);
 @*/

/*@
 requires SEQ_AT(SuperStruct)(SEQ_PREPEND(SuperStruct)(s, SEQ_NEW(SuperStruct)()), 0).a == 1;
 @*/
int bar (SuperStruct s) {
    return s.a + s.b;
}

