// clang-format off
// Test that quantifiers and bound-variables are transformed correctly
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// CHECK-DAG: [[sLit:@[^ ]+]] = private unnamed_addr constant [2 x i8] c"x\00"

// COM: Declarations are created and labeled with MD-nodes
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] double [[bvD:@"pallas.boundVar double"]](ptr)
// CHECK-DAG: declare !pallas.specLib [[bvMD:![0-9]+]] i64 [[bvI:@"pallas.boundVar i64"]](ptr)
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
// CHECK-DAG: call double [[bvD]](ptr [[sLit]])
// CHECK-DAG: call i64 [[bvI]](ptr [[sLit]])

/*@
requires _forall(_bvInt("x") >= 0 && _bvInt("x") < 10, _bvInt("x") >= 0);
requires _forallSep(_bvInt("x") >= 0 && _bvDouble("x") < 10, true);
requires _exists(_bvInt("x") >= 0, _bvInt("x") >= 0);
@*/
func funcThatReturnsAStruct(a: Int, b: Int) -> Int {
    return a + b
}
