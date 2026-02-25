// clang-format off
// Test that calls to predicates are encoded correctly in predicates
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: requires is encoded in MD
// CHECK: !pallas.fcontract [[contrMD:![0-9]+]]

// CHECK-DAG: [[contrMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]]}
// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", {{![0-9]+}}, ptr @_Z13PALLAS_SPEC_0Pii,

// COM: Call to the pred-function exists
// CHECK-DAG: = call noundef zeroext i1 @_Z4pArrPii(

/*@
declare using namespace pallasSpec;
@*/

/*@
predicate pArr(int *a, int n) :=  _forall(_inRange(0, _bv<int>("i"), n), 
                                         _Perm(&a[_bv<int>("i")], _write));
@*/

/*@
requires pArr(arr, size);
@*/
void zero_arr(int *arr, int size) {
    for (int i = 0; i < size; ++i) {
        arr[i] = 0;
    }
}