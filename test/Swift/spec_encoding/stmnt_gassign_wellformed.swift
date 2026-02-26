// clang-format off
// Test that an ghost-assignment is correctly encoded.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s


/*@
yields y: Int;
ensures y == 42;
@*/
func bar (a: Double) -> Double {
    /*@
    ghost assign y = 42;
    @*/
    return 1.0;
}


// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: MD-Node of the assert-statement is wellformed
// CHECK-DAG: [[blockMD]] = !{{{![0-9]+}}, [[gaMD:![0-9]+]]}
// CHECK-DAG: [[gaMD]] = !{!"pallas.gAssign", {{![0-9]+}}, ptr @{{"[^"]+"}}, [[emp:![0-9]+]], {{![0-9]+}}, [[vars:![0-9]+]], [[yDef:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[yDef]] = !{[[yLoc:![0-9]+]], !"y"}
// CHECK-DAG: [[yLoc]] = !{!"pallas.srcLoc", i64 7, i64 1, i64 7, i64 14, !{{[0-9]+}}}