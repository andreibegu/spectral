// clang-format off
// Test that the functions for creating new sequences and for getting the size
// of a sequence are encoded correctly.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Check that the type in the allocation has been replaced
// CHECK: {{%[0-9]+}} = alloca %pallas.seq.float

// COM: Check that declarations are present
// CHECK-DAG: declare !pallas.specLib [[sizeMD:![0-9]+]] i64 @"pallas.seq.size float"(ptr noundef byval(%pallas.seq.float))
// CHECK-DAG: declare !pallas.specLib [[newMD:![0-9]+]] void @"pallas.seq.new float"(ptr sret(%pallas.seq.float))

// COM: Check that the metadata-annotations are present
// CHECK-DAG: [[sizeMD]] = !{!"pallas.seq.size"}
// CHECK-DAG: [[newMD]] = !{!"pallas.seq.new"}

// COM: Check that the spec-functions are called
// CHECK-DAG: call void @"pallas.seq.new float"(ptr sret(%pallas.seq.float) %{{[0-9]+}})
// CHECK-DAG: call i64 @"pallas.seq.size float"(ptr noundef byval(%pallas.seq.float) %{{[0-9]+}})


/*@
declare #defSeq<Float>;
@*/

/*@
requires SeqFloat().size() >= 0;
@*/
func someFunction(a: Int, b: Int) -> Int {
    return a + b
}

