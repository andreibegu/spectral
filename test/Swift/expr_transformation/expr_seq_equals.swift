// clang-format off
// Test that the equality operator for sequences is encoded correctly
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Check that declaration is present
// CHECK-DAG: declare !pallas.specLib [[eqMD:![0-9]+]] i1 @pallas.seq.equals.0(ptr noundef byval(%pallas.seq.0), ptr noundef byval(%pallas.seq.0))

// COM: Check that the metadata-annotation is present
// CHECK-DAG: [[eqMD]] = !{!"pallas.seq.equals"}

// COM: Check that the spec-function is called
// CHECK-DAG: call i1 @pallas.seq.equals.0(ptr noundef byval(%pallas.seq.0) %{{[0-9]+}}, ptr noundef byval(%pallas.seq.0) %{{[0-9]+}})

/*@
declare #defSeq<Float>;
@*/

/*@
requires SeqFloat().equals(SeqFloat());
@*/
func bar(a: Int, b: Int) -> Int {
    return a + b
}

