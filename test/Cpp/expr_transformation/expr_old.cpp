// Test that expressions that include the old-construct are transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] noundef float @"pallas.old noundef float_noundef float"(float noundef)
// CHECK-DAG: declare !pallas.specLib [[oldMD]] noundef double @"pallas.old noundef double_noundef double"(double noundef)

// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called
// CHECK-DAG: = call noundef float @"pallas.old noundef float_noundef float"(float noundef
// CHECK-DAG: = call noundef double @"pallas.old noundef double_noundef double"(double noundef

/*@
ensures pallasSpec::_old(a) == a;
ensures pallasSpec::_old(b) == b;
@*/
void fun (double a, float b) {
    double c = a + b;
}