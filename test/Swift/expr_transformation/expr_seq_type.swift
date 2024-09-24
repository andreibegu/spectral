// clang-format off
// Test that the sequence-types are encoded correctly when multiple different
// types are used. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Check that only two types are generated (i.e. the float-type is not generated twice)
// CHECK-NOT: %pallas.seq.2

// CHECK-DAG: %pallas.seq.{{[0-9]+}} = type { float, i64, i64, i64 }
// CHECK-DAG: %pallas.seq.{{[0-9]+}} = type { double, i64, i64, i64 }


/*@
requires _seqSizeFloat(_seqNewFloat()) > 0 && _seqSizeDouble(_seqNewDouble()) > 0 ;
requires _seqSizeFloat(_seqNewFloat()) > 0 ;
@*/
func someFunction(a: Int, b: Int) -> Int {
    return a + b
}

