// clang-format off
// Test that ghost assignents are correctly encoded as MD-nodes. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

/*@
yields int y;
ensures y == 42;
@*/
float bar (float x) {
    /*@
    ghost assign y = 42;
    @*/
    return x + 1; 
}

// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: MD-Node of the ghost assign has the md-node of the yields-def appended
// CHECK-DAG: [[blockMD]] = !{{{![0-9]+}}, [[gaMD:![0-9]+]]}
// CHECK-DAG: [[gaMD]] = !{!"pallas.gAssign", {{![0-9]+}}, ptr @{{[^,]+}}, [[emp:![0-9]+]], {{![0-9]+}}, {{![0-9]+}}, [[yDef:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[yDef]] = !{[[yLoc:![0-9]+]], !"y"}
// CHECK-DAG: [[yLoc]] = !{!"pallas.srcLoc", i64 6, i64 1, i64 6, i64 13, !{{[0-9]+}}}