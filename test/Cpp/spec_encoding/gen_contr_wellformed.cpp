// clang-format off
// Test that generated contracts are encoded correctly.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

/*@
declare using namespace pallasSpec;
@*/

class CheerfulClass {
    public: 
    int i = 0;
    int j = 2; 

    // Contract for the generated constructor
    /*@
    assumed contract for generated src CheerfulClass;
    requires _sep(_Perm(&this->i, _write), _Perm(&this->j, _write)); 
    ensures _sep(_Perm(&this->i, _write), _Perm(&this->j, _write));
    ensures (this->i == 0) && (this->j == 2);
    @*/

};

int foo(int i, int j) {
    CheerfulClass c;
    c.i = 1;
    c.j = 3;
    return 42;
}

// CHECK: define {{.*}} void @_ZN13CheerfulClassC2Ev(ptr noundef nonnull align 4 dereferenceable(8) %0)
// CHECK-SAME: !pallas.fcontract [[contrMD:![0-9]+]]
// CHECK-DAG: [[contrMD]] = !{{{![0-9]+}}, i1 false, i1 true, [[reqMD:![0-9]+]], [[ens1MD:![0-9]+]], [[ens2MD:![0-9]+]]}
// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", {{![0-9]+}}, ptr @_ZN13CheerfulClass13PALLAS_SPEC_0Ev, [[thisMD:![0-9]+]]}
// CHECK-DAG: [[thisMD]] = !DILocalVariable(name: "this", arg: 1,