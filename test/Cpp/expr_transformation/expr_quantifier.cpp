// Test that quantifiers are transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// COM: Old declarations not present (Might not work with different name-mangling)
// CHECK-NOT: @_ZN10pallasSpec4bVarIdEET_PKc(
// CHECK-NOT: @_ZN10pallasSpec6existsEbb
// CHECK-NOT: @_ZN10pallasSpec6forallEbb
// CHECK-NOT: @_ZN10pallasSpec9forallSepEbb

// COM: Declarations are created and labeled with MD-nodes
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] double [[bv:@pallas.boundVar.[0-9]+]](ptr)
// CHECK-DAG: [[bvMD]] = !{!"pallas.boundVar"}
// CHECK-DAG: declare !pallas.specLib [[exMD:![0-9]+]] i1 @pallas.exists(i1, i1)
// CHECK-DAG: [[exMD]] = !{!"pallas.exists"}
// CHECK-DAG: declare !pallas.specLib [[allMD:![0-9]+]] i1 @pallas.forall(i1, i1)
// CHECK-DAG: [[allMD]] = !{!"pallas.forall"}
// CHECK-DAG: declare !pallas.specLib [[allSepMD:![0-9]+]] i1 @pallas.forallSep(i1, i1)
// CHECK-DAG: [[allSepMD]] = !{!"pallas.forallSep"}

// COM: spec-functions are called
// CHECK-DAG: call i1 @pallas.exists(
// CHECK-DAG: call i1 @pallas.forall(
// CHECK-DAG: call i1 @pallas.forallSep(
// CHECK-DAG: call double [[bv]](


/*@
 ensures pallasSpec::exists(0 <= pallasSpec::bVar<double>("x") && pallasSpec::bVar<double>("x") <= 10,  pallasSpec::bVar<double>("x") != 42);
 ensures pallasSpec::forall(0 <= pallasSpec::bVar<double>("i"),  true);
 ensures pallasSpec::forallSep(0 <= pallasSpec::bVar<double>("i"),  true);
@*/
double fun (double a) {
    return a > 0 ? a : -1 * a;
}



