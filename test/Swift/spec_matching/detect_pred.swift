// clang-format off
// Test that predicate definitions are correctly detected. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s


class MyTuple {
    var fst : Int = 0
    var snd : Int = 0
}

/*@
predicate tupRead(_ t : MyTuple) := 
                                    _perm(t, Frac(1, 2));
@*/

// CHECK: Found predicate at 12::1 - 13::57
