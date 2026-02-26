// clang-format off
// Test that contracts of ghost functions are correctly encoded and 
// that ghost-function can be called in specifications. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m | %FileCheck %s

/*@
declare using namespace pallasSpec;
@*/

namespace specs {

/*@
ghost function
requires i > 0 && j > 0;
int foo(int i, int j) { return 6 * i + 3 * j; }
@*/

}

/*@
ensures specs::foo(1, 4) == 18;
@*/
int run() {
   return 42; 
}

// COM: Check that Definition of ghost function is present, marked and has contract
// CHECK: define dso_local noundef i32 @_ZN5specs3fooEii( 
// CHECK-SAME: !pallas.ghost [[ghostMarkerMD:![0-9]+]] 
// CHECK-SAME: !pallas.fcontract [[ghostContrMD:![0-9]+]]
// CHECK-DAG: [[ghostMarkerMD]] = !{!""}

// COM: Get the DIVariables of foo
// CHECK-DAG: [[fooSP:![0-9]+]] = distinct !DISubprogram(name: "foo",
// CHECK-DAG: [[fooI:![0-9]+]] = !DILocalVariable(name: "i", arg: 1, scope: [[fooSP]]
// CHECK-DAG: [[fooJ:![0-9]+]] = !DILocalVariable(name: "j", arg: 2, scope: [[fooSP]]


// COM: Check that the contract of the ghost function is wellformed:
// CHECK-DAG: [[ghostContrMD]] = !{[[contrLocMD:![0-9]+]], i1 true, i1 false, [[emp:![0-9]+]], [[emp]], [[requiresMD:![0-9]+]]}
// CHECK-DAG: [[contrLocMD]] = !{!"pallas.srcLoc", i64 14, i64 1, i64 14, i64 24, [[fileMD:![0-9]+]]}
// CHECK-DAG: [[requiresMD]] = !{!"pallas.requires", [[contrLocMD:![0-9]+]], ptr @_ZN5specs13[[wrapper:PALLAS_SPEC_[0-9]]]Eii, [[emp]], [[emp]], [[vars:![0-9]+]]}

// COM: Get the DIVariables of the wrapper
// CHECK-DAG: [[w1SP:![0-9]+]] = distinct !DISubprogram(name: "[[wrapper]]
// CHECK-DAG: [[w1I:![0-9]+]] = !DILocalVariable(name: "i", arg: {{[0-9]}}, scope: [[w1SP]]
// CHECK-DAG: [[w1J:![0-9]+]] = !DILocalVariable(name: "j", arg: {{[0-9]}}, scope: [[w1SP]]

// CHECK-DAG: [[vars]] = !{[[m1:![0-9]+]], [[m2:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[m1]] = !{[[fooI]], [[w1I]]}
// CHECK-DAG: [[m2]] = !{[[fooJ]], [[w1J]]}