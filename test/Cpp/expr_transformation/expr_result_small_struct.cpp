// clang-format off
// Test that the result-construct is transformed correctly for structs
// that are passed and retrned as a single int. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] i64 @"pallas.result i64"()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call i64 @"pallas.result i64"()
#include <cstdint>

struct SillyStruct {
    int32_t a;
    int32_t b;
};

/*@
ensures pallasSpec::_result<SillyStruct>().a >= 0;
@*/
SillyStruct fun (SillyStruct s) {
    s.a = 0;
    return s;
}