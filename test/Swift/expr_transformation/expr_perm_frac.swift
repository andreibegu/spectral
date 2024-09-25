// clang-format off
// Test that the datatype for fraction, the function for creating fractions and 
// the Perm-function are encoded correctly. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s


// COM: Old fraction-type is not present
// CHECK-NOT: %T10PallasSpec4FracV

// COM: Fraction-type is present
// CHECK-DAG: %pallas.fracT = type { i64, i64, i64, i64 }

// COM: Declaration of fracOf and Perm is present 
// CHECK-DAG: declare !pallas.specLib [[fracMD:![0-9]+]] void @pallas.fracOf(ptr sret(%pallas.fracT), i32 noundef, i32 noundef)
// CHECK-DAG: declare !pallas.specLib [[permMD:![0-9]+]] i1 @pallas.perm(ptr noundef, ptr noundef byval(%pallas.fracT))
// CHECK-DAG: [[fracMD]] = !{!"pallas.fracOf"}
// CHECK-DAG: [[permMD]] = !{!"pallas.perm"}

// COM: Specification-functions are called 
// CHECK-DAG: call void @pallas.fracOf(ptr sret(%pallas.fracT) {{%[0-9]+}}, i32 noundef 1, i32 noundef 2)
// CHECK-DAG: call i1 @pallas.perm(ptr noundef {{%[0-9]+}}, ptr noundef byval(%pallas.fracT) {{%[0-9]+}})



public class Clazz {
    var placeholder1: Int64 = 0
    var placeholder2: Int64 = 0
    var placeholder3: Int64 = 0
    var placeholder4: Int64 = 0
    var placeholder5: Int64 = 0
}


/*@
requires _perm(a, Frac(1, 2));
@*/
func functionWithPermissions(a: Clazz, b: Clazz) {
    let c = a.placeholder1 + b.placeholder1
    let d = a.placeholder2 + b.placeholder2
}
