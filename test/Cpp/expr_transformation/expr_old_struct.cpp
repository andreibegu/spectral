// clang-format off
// Test that expressions that include the old-construct are transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created and has two arguments
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] void @pallas.old.0(ptr sret(%struct.SuperStruct), ptr byval(%struct.SuperStruct))

// COM: Declaration has MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-function is called
// CHECK-DAG: call void @pallas.old.0(ptr sret(%struct.SuperStruct) {{%[0-9]+}}, ptr byval(%struct.SuperStruct) {{%[0-9]+}})


struct SuperStruct {
    int a, b, c, d, e, f, g;
};

/*@
ensures pallasSpec::old(s).a == s.a;
@*/
void fun (SuperStruct s) {
    int c = s.a + s.b;
}