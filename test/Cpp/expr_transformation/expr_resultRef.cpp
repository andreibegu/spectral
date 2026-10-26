// clang-format off
// Test that the reference-variant of the result-construct is transformed correctly.
// Also checks that for types with a non-trivial destructor no destructo-call is generated 
// in the wrapper function.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s --check-prefix=CHECK2

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] ptr @pallas.resultRef()
// CHECK-DAG: [[resMD]] = !{!"pallas.resultRef"}
// COM: call is present and attributes are copied during unification
// CHECK-DAG: call noundef nonnull align 8 dereferenceable(16) ptr @pallas.resultRef()

// COM: No destructor call present in wrapper
// CHECK2-LABEL: define {{.*}} @_Z13PALLAS_SPEC_0d(
// CHECK2-NOT: @_ZN11SillyStructD1Ev
// CHECK2: ret i1

struct SillyStruct {
    double a, b;
    ~SillyStruct();
};

/*@
ensures pallasSpec::_resultRef<SillyStruct>().a >= 0;
@*/
SillyStruct fun (double a) {
    SillyStruct s;
    s.a = a;
    return s;
}
