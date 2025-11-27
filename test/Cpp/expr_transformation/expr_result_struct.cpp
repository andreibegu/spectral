// clang-format off
// Test that the result-construct is transformed correctly for structs
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] void @"pallas.result sret(%struct.SillyStruct) align 8 void"(ptr sret(%struct.SillyStruct) align 8)
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call void @"pallas.result sret(%struct.SillyStruct) align 8 void"(ptr sret(%struct.SillyStruct) align 8 {{%[0-9]+}})

struct SillyStruct {
    double a, b, c, d, e, f, g;
};

/*@
ensures pallasSpec::_result<SillyStruct>().a >= 0;
@*/
SillyStruct fun (SillyStruct s) {
    s.a = 0;
    return s;
}