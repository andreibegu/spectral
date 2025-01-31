// Check that expressions with specifications about pointer-blocks get transformed
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Pointer block length:
// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[pblMD:![0-9]+]] i64 @pallas.ptrBlockLength(ptr noundef)
// CHECK-DAG: [[pblMD]] = !{!"pallas.ptrBlockLength"}
// COM: Function is called
// CHECK-DAG: call i64 @pallas.ptrBlockLength(ptr noundef {{%[0-9]+}})

// COM: Pointer block offset:
// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[pboMD:![0-9]+]] i64 @pallas.ptrBlockOffset(ptr noundef)
// CHECK-DAG: [[pboMD]] = !{!"pallas.ptrBlockOffset"}
// COM: Function is called
// CHECK-DAG: call i64 @pallas.ptrBlockOffset(ptr noundef {{%[0-9]+}})

// COM: Pointer length:
// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[plMD:![0-9]+]] i64 @pallas.ptrLength(ptr noundef)
// CHECK-DAG: [[plMD]] = !{!"pallas.ptrLength"}
// COM: Function is called
// CHECK-DAG: call i64 @pallas.ptrLength(ptr noundef {{%[0-9]+}})

/*@
 requires arr != nullptr && n >= 0;
 requires pallasSpec::ptrBlockLength(arr) == n && 
          pallasSpec::ptrBlockOffset(arr) == 0 &&
          pallasSpec::ptrLength(arr) == n;
 @*/
void clear (int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = 0;
    }
}
