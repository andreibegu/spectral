// clang-format off
// Test that the result-construct is transformed correctly for structs
// that are passed as multiple ints and returned as a compound-value. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] { i64, i32 } @"pallas.result { i64, i32 }"()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call { i64, i32 } @"pallas.result { i64, i32 }"()
#include <cstdint>

struct SillyStruct {
    int32_t a;
    int32_t b;
    int32_t c;
};

/*@
ensures pallasSpec::_result<SillyStruct>().a >= 0;
@*/
SillyStruct fun (SillyStruct s) {
    s.a = 0;
    return s;
}