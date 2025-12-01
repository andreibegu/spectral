// clang-format off
// Test that bound-variables work properly with struct-types when the 
// struct is passed as multiple integers and returned as a compound-value. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created and labled with md-node
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] { i64, i64 } @"pallas.boundVar { i64, i64 }"(ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}

// COM: Spec-function is called with sret-attribute
// CHECK-DAG call { i64, i64 } @"pallas.boundVar { i64, i64 }"(ptr {{%[0-9]+}})

#include <cstdint>

class ClumsyClass {
    public:
    int16_t i1;
    int16_t i2;
    int64_t i3;
};

/*@
 ensures pallasSpec::_forall(0 <= pallasSpec::_bv<ClumsyClass>("i").i1, true);
@*/
int fun (ClumsyClass c) {
    return c.i1 + c.i2;
}

