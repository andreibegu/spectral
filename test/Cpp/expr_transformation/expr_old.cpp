// Test that expressions that include the old-construct are transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] noundef float [[oldF:@pallas.old.[0-9]+]](float noundef)
// CHECK-DAG: declare !pallas.specLib [[oldMD]] noundef double [[oldD:@pallas.old.[0-9]+]](double noundef)

// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called
// CHECK-DAG: = call noundef float [[oldF]](float noundef
// CHECK-DAG: = call noundef double [[oldD]](double noundef

/*@
ensures pallasSpec::old(a) == a;
ensures pallasSpec::old(b) == b;
@*/
void fun (double a, float b) {
    double c = a + b;
}