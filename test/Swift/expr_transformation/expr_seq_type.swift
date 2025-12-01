// clang-format off
// Test that the sequence-types are encoded correctly when multiple different
// types are used. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Check that only two types are generated (i.e. the float-type is not generated twice)
// CHECK-NOT: %pallas.seq.2

// CHECK-DAG: %pallas.seq.float = type { float, i64, i64, i64 }
// CHECK-DAG: %pallas.seq.double = type { double, i64, i64, i64 }

/*@
declare #defSeq<Float>;
declare #defSeq<Double>;
@*/

/*@
requires SeqFloat().size() >= 0 && SeqDouble().size() >= 0;
requires SeqFloat().size() >= 0;
@*/
func someFunction(a: Int, b: Int) -> Int {
    return a + b
}

