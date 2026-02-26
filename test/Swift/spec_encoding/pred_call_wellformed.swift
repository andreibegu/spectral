// clang-format off
// Test that an assert-statement is correctly encoded.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

class MyTuple {
    var fst : Int = 0
    var snd : Int = 0
}

/*@
predicate tupRead(_ t : MyTuple) := _perm(t, Frac(1, 2));
@*/

/*@
requires tupRead(t);
@*/
func swapTuple(_ t : MyTuple) -> MyTuple {
    let res = MyTuple()
    res.fst = t.snd
    res.snd = t.fst
    return res
}


// COM: requires is present
// CHECK: !pallas.fcontract [[contrMD:![0-9]+]]

// CHECK-DAG: [[contrMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]]}
// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", {{![0-9]+}}, ptr @"{{.*}}PALLAS_SPEC_0{{.*}}",
// CHECK-DAG: [[emp]] = !{}

// CHECK-DAG: define hidden swiftcc i1 @"{{.*}}tupReady{{.*}}"(ptr %0) {{#[0-9]+}} !dbg {{![0-9]+}} !pallas.predDef

// COM: Call to the pred-function exists
// CHECK-DAG: = call swiftcc i1 @"{{.*}}tupReady{{.*}}"(ptr %0)
