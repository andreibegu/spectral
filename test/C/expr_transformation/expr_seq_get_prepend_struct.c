// clang-format off
// Test that the get- and prepend-functions for sequences are encoded correctly
// when used with struct-types. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[getMD:![0-9]+]] void @"pallas.seq.get %struct.S_sret(%struct.S) align 4 void"(ptr sret(%struct.S) align 4, ptr noundef byval(%"pallas.seq.%struct.S"), i64 noundef)
// CHECK-DAG: declare !pallas.specLib [[preMD:![0-9]+]] void @"pallas.seq.prepend %struct.S_noundef byval(%struct.S) align 8 ptr"(ptr sret(%"pallas.seq.%struct.S"), ptr noundef byval(%struct.S) align 8, ptr noundef byval(%"pallas.seq.%struct.S"))


// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[getMD]] = !{!"pallas.seq.get"}
// CHECK-DAG: [[preMD]] = !{!"pallas.seq.prepend"}

// COM: Check that the spec-functions are called with the corret attributes
// CHECK-DAG: call void @"pallas.seq.prepend %struct.S_noundef byval(%struct.S) align 8 ptr"(ptr sret(%"pallas.seq.%struct.S") %{{[0-9]+}}, ptr noundef byval(%struct.S) align 8 {{%[0-9]+}}, ptr noundef byval(%"pallas.seq.%struct.S") %{{[0-9]+}})
// CHECK-DAG: call void @"pallas.seq.get %struct.S_sret(%struct.S) align 4 void"(ptr sret(%struct.S) align 4 %{{[0-9]+}}, ptr noundef byval(%"pallas.seq.%struct.S") %{{[0-9]+}}, i64 noundef 0)

typedef struct S {
    int a, b, c, d, e, f, g;
} SuperStruct;

/*@
 declare DEF_SEQ(SuperStruct);
 @*/

/*@
 requires _seqAt(SuperStruct)(_seqPrepend(SuperStruct)(s, _seqNew(SuperStruct)()), 0).a == 1;
 @*/
int bar (SuperStruct s) {
    return s.a + s.b;
}

