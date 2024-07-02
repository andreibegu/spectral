// Test that ensures-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | FileCheck-15 %s

// Contract is attached to function
// COM: CHECK: define
// COM: CHECK-SAME: !dbg [[funcMD:![0-9]+]] !pallas.fcontract [[contrMD:![0-9]+]]

// Generated specification-function is present

// Contract is attached to function
// CHECK-DAG: [[funcMD:![0-9]+]] = distinct !DISubprogram(name: "bar"
// CHECK-DAG: !dbg [[funcMD]] !pallas.fcontract [[contractMD:![0-9]+]]

// COM: Specification MD-nodes are wellformed
// CHECK-DAG: [[contractMD]] = !{[[ensuresMD:![0-9]+]]}
// CHECK-DAG: [[ensuresMD]] = !{!"pallas.ensures", ptr [[wrapperFunc:@.+]], [[varMD:![0-9]+]]}
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %0, metadata [[varMD]]

// COM: Wrapper-function is annotated
// CHECK-DAG: [[wrapperFunc]](i32 noundef %0) {{.+}} !dbg {{.+}} !pallas.exprWrapper

/*@
ensures x >= 0;
@*/
int bar (int x) {
    return x; 
}