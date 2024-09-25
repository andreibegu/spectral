// Test that checking for permissions and creating fractions 
// is encoded correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Fraction-type is present
// CHECK-DAG: %pallas.fracT = type { i64, i64, i64, i64 }

// COM: Declaration of fracOf and Perm is present 
// CHECK-DAG: declare !pallas.specLib [[permMD:![0-9]+]] i1 @pallas.perm(ptr noundef, ptr noundef byval(%pallas.fracT))
// CHECK-DAG: declare !pallas.specLib [[fracMD:![0-9]+]] void @pallas.fracOf(ptr sret(%pallas.fracT), i32 noundef, i32 noundef)
// CHECK-DAG: [[permMD]] = !{!"pallas.perm"}
// CHECK-DAG: [[fracMD]] = !{!"pallas.fracOf"}

// COM: Specification-functions are called 
// CHECK-DAG: call void @pallas.fracOf(ptr sret(%pallas.fracT)
// CHECK-DAG: call i1 @pallas.perm(


/*@
 requires Perm(a, fracOf(1, 2));
 @*/
void bar (int *a, float b) {
    float c = *a + b;
}