// Check that expressions with const-values get transformed
// RUN: %Spec2IR %s -lib %CSpecLib -m | FileCheck-15 %s

// Check that a and b are maked as const
// I.e. the dbg.value intrinsict point to a DILocalVariable that points to a
// type that has the correct const-information
// CHECK: @PALLAS_SPEC_0(ptr noundef [[pA:%[0-9]+]], ptr noundef [[pB:%[0-9]+]])
// CHECK: call void @llvm.dbg.value(metadata ptr [[pA]], metadata [[aMD:![0-9]+]]
// CHECK: call void @llvm.dbg.value(metadata ptr [[pB]], metadata [[bMD:![0-9]+]]

// CHECK-DAG: [[floatT:![0-9]+]] = !DIBasicType(name: "float"

// COM: Check type of a (i.e. a pointer to a const float)
// CHECK-DAG: [[aMD]] = !DILocalVariable({{.*}}, type: [[aT:![0-9]+]])
// CHECK-DAG: [[aT]] = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: [[aT1:![0-9]+]]
// CHECK-DAG: [[aT1]] = !DIDerivedType(tag: DW_TAG_const_type, baseType: [[floatT]]

// COM: Check type of b (i.e. a const pointer to a float)
// CHECK-DAG: [[bMD]] = !DILocalVariable({{.*}}, type: [[bT:![0-9]+]])
// CHECK-DAG: [[bT]] = !DIDerivedType(tag: DW_TAG_const_type, baseType: [[bT1:![0-9]+]]
// CHECK-DAG: [[bT1]] = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: [[floatT]]


/*@
requires *a == *b;
@*/
float foo (const float *a, float * const b){
    return *a + *b;
}