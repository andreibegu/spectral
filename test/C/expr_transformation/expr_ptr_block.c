// Check specification of pointer-block properties are correctly transformed
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

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
 requires arr != NULL && n >= 0;
 requires _ptr_block_length(arr) == n && _ptr_block_offset(arr) == 0;
 requires _ptr_length(arr) == n;
 @*/
void clear (int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = 0;
    }
}
