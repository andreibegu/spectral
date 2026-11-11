// clang-format off
// Test that the transitive-assume-flag for generated contracts are encoded correctly.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

/*@
declare using namespace pallasSpec;
@*/

class CheesyClass {
    public: 
    int a = 0; 

    // Contract for the generated constructor
    /*@
    transitively assumed contract for generated src CheesyClass;
    requires _Perm(&*this, _writePerm);
    ensures  _Perm(&*this, _writePerm);
    ensures this->a == 0;
    @*/

};

int foo(int i, int j) {
    CheesyClass c;
    c.a = 1 + i + j;
    return c.a;
}

// CHECK: define {{.*}} void @_ZN11CheesyClassC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %0)
// CHECK-SAME: !pallas.fcontract [[contrMD:![0-9]+]]
// CHECK-DAG: [[contrMD]] = !{{{![0-9]+}}, i1 false, i8 2, [[emp:![0-9]+]], [[emp]], {{![0-9]+}}, {{![0-9]+}}, {{![0-9]+}}}
// CHECK-DAG: [[emp]] = !{}