// Test that quantifiers are transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
// COM: Old declarations not present
// CHECK-NOT: PALLAS_BOUND_VAR___
// CHECK-NOT: exists
// CHECK-NOT: forall
// CHECK-NOT: forallS
// COM: Declarations are created and labeled with MD-nodes
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] double @"pallas.boundVar double"(ptr)
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
// CHECK-DAG: call double @"pallas.boundVar double"(


/*@
 declare DEF_BV(double);
@*/

/*@
 ensures _exists(0 <= _bv(double, x) && _bv(double, x) <= 10,  _bv(double, x) != 42);
 ensures _forall(0 <= _bv(double, i),  true);
 ensures _forallS(0 <= _bv(double, i),  true);
@*/
double fun (double a) {
    return a > 0 ? a : -1 * a;
}



