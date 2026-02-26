// clang-format off
// Test that bindings to ghost-arguments are encoded correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m | %FileCheck %s

/*@
given int g;
yields int y;
requires x >= 0 && g > 0;
ensures y > 0;
@*/
int foo (int x) {
    return x; 
}

/*@
yields int r;
ensures r != 42;
@*/
void run() {
    int i = 9;
    foo /*@ given g = 1; @*/ /*@ yields r = y; @*/ (i);

}

// COM: Get ghost-arg definitions of foo
// CHECK-DAG: define dso_local noundef i32 @_Z3fooi({{.*}} !pallas.fcontract [[fooContr:![0-9]+]] 
// CHECK-DAG: [[fooContr]] = !{!{{[0-9]+}}, i1 false, i1 false, [[fooGList:![0-9]+]], [[fooYList:![0-9]+]], !{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-DAG: [[fooGList]] = !{[[fooGDef:![0-9]+]]}
// CHECK-DAG: [[fooGDef]] = !{!{{[0-9]+}}, !"g"}
// CHECK-DAG: [[fooYList]] = !{[[fooYDef:![0-9]+]]}
// CHECK-DAG: [[fooYDef]] = !{!{{[0-9]+}}, !"y"}

// COM: Get ghost-arg definitions of run
// CHECK-DAG: define dso_local void @_Z3runv({{.*}} !pallas.fcontract [[runContr:![0-9]+]]
// CHECK-DAG: [[runContr]] = !{!{{[0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[runYList:![0-9]+]], !{{[0-9]+}}}
// CHECK-DAG: [[runYList]] = !{[[runYDef:![0-9]+]]}
// CHECK-DAG: [[runYDef]] = !{!{{[0-9]+}}, !"r"}
// CHECK-DAG: [[emp]] = !{}



// COM: Bindings are attached to the call
// CHECK-DAG: call noundef i32 @_Z3fooi({{.*}} !pallas.givenBindings [[gBindMD:![0-9]+]], !pallas.yieldsBindings [[yBindMD:![0-9]+]]

// COM: Given-binding is encoded correctly:
// CHECK-DAG: [[gBindMD]] = !{[[gBindLoc:![0-9]+]], [[gBindEntry:![0-9]+]]}
// CHECK-DAG: [[gBindLoc]] = !{!"pallas.srcLoc", i64 21, i64 9, i64 21, i64 26, !{{[0-9]+}}}
// CHECK-DAG: [[gBindEntry]] = !{!"pallas.givenBinding", [[gBELoc:![0-9]+]], ptr [[gbEWrapper:@_Z13PALLAS_SPEC_[0-9]+ii]], !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, [[fooGDef]]}
// CHECK-DAG: [[gBELoc]] = !{!"pallas.srcLoc", i64 21, i64 19, i64 21, i64 24, !{{[0-9]+}}}


// COM: Yields-binding is encoded correctly
// CHECK-DAG: [[yBindMD]] = !{[[yBindLoc:![0-9]+]], [[yBindEntry:![0-9]+]]}
// CHECK-DAG: [[yBindLoc]] = !{!"pallas.srcLoc", i64 21, i64 30, i64 21, i64 48, !{{[0-9]+}}}
// CHECK-DAG: [[yBindEntry]] = !{!"pallas.yieldsBinding", [[yBELoc:![0-9]+]], [[runYDef]], [[fooYDef]]}
// CHECK-DAG: [[yBELoc]] = !{!"pallas.srcLoc", i64 21, i64 41, i64 21, i64 46, !{{[0-9]+}}}
