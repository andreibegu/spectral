// clang-format off
// Test that bound-variables work properly with struct-types when the 
// struct is treated as a single integer. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created and labled with md-node
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] i32 @"pallas.boundVar i32"(ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}

// COM: Spec-function is called with sret-attribute
// CHECK-DAG call i32 @"pallas.boundVar i32"(ptr {{%[0-9]+}})

#include <cstdint>

class ClumsyClass {
    public:
    int16_t i1;
    int16_t i2;
};

/*@
 ensures pallasSpec::_forall(0 <= pallasSpec::_bv<ClumsyClass>("i").i1, true);
@*/
int fun (ClumsyClass c) {
    return c.i1 + c.i2;
}

