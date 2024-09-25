// Test that quantifiers are transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
// COM: Old declarations not present
// CHECK-NOT: PALLAS_BOUND_VAR___
// CHECK-NOT: exists
// CHECK-NOT: forall
// CHECK-NOT: forallP
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
 declare DEF_BV(double);
@*/

/*@
 ensures exists(0 <= BV(double)("x") && BV(double)("x") <= 10,  BV(double)("x") != 42);
 ensures forall(0 <= BV(double)("i"),  true);
 ensures forallP(0 <= BV(double)("i"),  true);
@*/
double fun (double a) {
    return a > 0 ? a : -1 * a;
}



