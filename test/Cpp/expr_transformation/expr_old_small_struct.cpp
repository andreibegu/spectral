// clang-format off
// Test that expressions that include the old-construct are transformed correctly
// when used with small structs, that are passed and returned as a single int.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created and has two arguments
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] i48 @"pallas.old i48_i48"(i48)

// COM: Declaration has MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-function is called
// CHECK-DAG: call i48 @"pallas.old i48_i48"(i48 {{%[0-9]+}})
#include <cstdint>

struct SuperStruct {
    int16_t a;
    int16_t b;
    int16_t c;
};

/*@
ensures pallasSpec::_old(s).a == s.a;
@*/
void fun (SuperStruct s) {
    int c = s.a + s.b;
}