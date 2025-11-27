// clang-format off
// Test that bound-variables work properly with struct-types.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created and labled with md-node
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] void @"pallas.boundVar sret(%class.ClumsyClass) align 8 void"(ptr sret(%class.ClumsyClass) align 8, ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}

// COM: Spec-function is called with sret-attribute
// CHECK-DAG call void @"pallas.boundVar sret(%class.ClumsyClass) align 8 void"(ptr sret(%class.ClumsyClass) align 8 {{%[0-9]+}}, ptr {{%[0-9]+}})

class ClumsyClass {
    public:
    int i1, i2, i3, i4, i5;
    double d1, d2, d3;
};

/*@
 ensures pallasSpec::_forall(0 <= pallasSpec::_bv<ClumsyClass>("i").i1,  true);
@*/
double fun (ClumsyClass c) {
    return c.d1 + c.d2;
}

