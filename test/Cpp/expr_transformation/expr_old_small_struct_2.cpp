// clang-format off
// Test that expressions that include the old-construct are transformed correctly
// when used with small structs, that are passed as multiple integers and 
// returned as a compound-value. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created and has two arguments
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] { i64, i16 } @"pallas.old { i64, i16 }_i64,i16"(i64, i16)

// COM: Declaration has MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-function is called
// CHECK-DAG: call { i64, i16 } @"pallas.old { i64, i16 }_i64,i16"(i64 {{%[0-9]+}}, i16 {{%[0-9]+}})
#include <cstdint>

struct SuperStruct {
    int64_t a;
    int16_t b;
};

/*@
ensures pallasSpec::_old(s).a == s.a;
@*/
void fun (SuperStruct s) {
    int c = s.a + s.b;
}