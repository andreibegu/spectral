// clang-format off
// Test that the _unfolding construct is translated correctly
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declarations of _unfolding is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] i1 @"pallas.unfolding i1_i1"(i1 noundef zeroext, i1)
// CHECK-DAG: declare !pallas.specLib [[mdNode]] i64 @"pallas.unfolding i64_i64"(i1 noundef zeroext, i64)
// CHECK-DAG: [[mdNode]] = !{!"pallas.unfolding"}

// COM: Unified specification-functions are called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @"pallas.unfolding i1_i1"(
// CHECK-DAG: {{%[0-9]+}} = call i64 @"pallas.unfolding i64_i64"(
class MyTuple {
    var fst : Int = 0
    var snd : Int = 0
}

/*@
predicate tupRead(_ t : MyTuple) := _perm(t, Frac(1, 2));
@*/

/*@
requires tupRead(t);
requires _unfolding(tupRead(t), t.fst == 0);
requires _unfoldingInt(tupRead(t), t.fst) == 0;
@*/
func swapTuple(_ t : MyTuple) -> MyTuple {
    let res = MyTuple()
    res.fst = t.snd
    res.snd = t.fst
    return res
}