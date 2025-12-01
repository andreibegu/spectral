// clang-format off
// Test that expressions that include the old-construct are transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created and has two arguments
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] void @"pallas.old sret(%struct.SuperStruct) align 4 void_noundef byval(%struct.SuperStruct) align 8 ptr"(ptr sret(%struct.SuperStruct) align 4, ptr noundef byval(%struct.SuperStruct) align 8)

// COM: Declaration has MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-function is called
// CHECK-DAG: call void @"pallas.old sret(%struct.SuperStruct) align 4 void_noundef byval(%struct.SuperStruct) align 8 ptr"(ptr sret(%struct.SuperStruct) align 4 {{%[0-9]+}}, ptr noundef byval(%struct.SuperStruct) align 8 {{%[0-9]+}})


struct SuperStruct {
    int a, b, c, d, e, f, g;
};

/*@
ensures pallasSpec::_old(s).a == s.a;
@*/
void fun (SuperStruct s) {
    int c = s.a + s.b;
}