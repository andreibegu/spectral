// clang-format off
// Test that an external contract that is applied to a function definition is 
//  encoded correctly. I.e. it is encoded as a regular contract and supports 
//  renaming arguments.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s
#include <stdint.h>

/*@
declare using namespace pallasSpec;
@*/

struct SomeStruct {
    int64_t a;
    int64_t b = 0;

    // Force default-destructor to be created
    ~SomeStruct() {}
};

/*@
contract for external ir _ZN10SomeStructD2Ev;
args SomeStruct* _this;
returns void;
requires _sep(_this != nullptr, 
              _Perm(_this, _writePerm));
@*/


int main () {
    SomeStruct s;
    s.a = 0;
    s.b= 0;
}


// Check that contract is attached to function
// CHECK: define {{.*}} void @_ZN10SomeStructD2Ev(
// CHECK-SAME: %0)
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// Get MD-Node for the argument in parent-function: 
// CHECK: store ptr %0, ptr [[iVal:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[iVal]], metadata [[pArgMD:![0-9]+]]

// Check that wrapper is labled:
// CHECK: define {{.*}} i1 @_Z13PALLAS_SPEC_0P10SomeStruct(ptr noundef %0) 
// CHECK-SAME: !pallas.exprWrapper
// CHECK: call void @llvm.dbg.value(metadata ptr %0, metadata [[wArgMD:![0-9]+]]

// Check encoding of the contract:
// CHECK-DAG: [[contractMD]] = !{{{![0-9]+}}, i1 false, i8 0, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", {{![0-9]+}}, ptr @_Z13PALLAS_SPEC_0P10SomeStruct, [[emp]], [[emp]], [[argMap:![0-9]+]]}
// CHECK-DAG: [[argMap]] = !{[[thisMap:![0-9]+]]}
// CHECK-DAG: [[thisMap]] = !{[[pArgMD]], [[wArgMD]]}