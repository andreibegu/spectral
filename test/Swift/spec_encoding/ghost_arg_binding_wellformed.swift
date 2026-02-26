// clang-format off
// Test that definitions of ghost arguments are wellformed.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

/*@
given g: Int;
yields y: Int;
requires x >= 0 && g > 0;
ensures y > 0;
@*/
func foo(_ x: Int) -> Int {
    return x; 
}

/*@
yields r: Int;
ensures r != 42;
@*/
func run() -> () {
    let i: Int = 9;
    _ = foo /*@ given g = 1; @*/ /*@ yields r = y; @*/ (i);

}


// COM: Get names of functions
// CHECK-DAG: [[fooSP:![0-9]+]] = distinct !DISubprogram(name: "foo", linkageName: [[fooLName:"[^"]+"]], 
// CHECK-DAG: [[runSP:![0-9]+]] = distinct !DISubprogram(name: "run", linkageName: [[runLName:"[^"]+"]],

// COM: Get ghost-arg definitions of foo
// CHECK-DAG: !dbg [[fooSP]] !pallas.fcontract [[fooContr:![0-9]+]] 
// CHECK-DAG: [[fooContr]] = !{!{{[0-9]+}}, i1 false, i1 false, [[fooGList:![0-9]+]], [[fooYList:![0-9]+]], !{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-DAG: [[fooGList]] = !{[[fooGDef:![0-9]+]]}
// CHECK-DAG: [[fooGDef]] = !{!{{[0-9]+}}, !"g"}
// CHECK-DAG: [[fooYList]] = !{[[fooYDef:![0-9]+]]}
// CHECK-DAG: [[fooYDef]] = !{!{{[0-9]+}}, !"y"}

// COM: Get ghost-arg definitions of run
// CHECK-DAG: !dbg [[runSP]] !pallas.fcontract [[runContr:![0-9]+]] 
// CHECK-DAG: [[runContr]] = !{!{{[0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[runYList:![0-9]+]], !{{[0-9]+}}}
// CHECK-DAG: [[runYList]] = !{[[runYDef:![0-9]+]]}
// CHECK-DAG: [[runYDef]] = !{!{{[0-9]+}}, !"r"}
// CHECK-DAG: [[emp]] = !{}


// COM: Bindings are attached to the call
// CHECK-DAG: call swiftcc i64 @[[fooLName]]({{.*}} !pallas.givenBindings [[gBindMD:![0-9]+]], !pallas.yieldsBindings [[yBindMD:![0-9]+]]

// COM: Given-binding is encoded correctly:
// CHECK-DAG: [[gBindMD]] = !{[[gBindLoc:![0-9]+]], [[gBindEntry:![0-9]+]]}
// CHECK-DAG: [[gBindLoc]] = !{!"pallas.srcLoc", i64 21, i64 13, i64 21, i64 30, !{{[0-9]+}}}
// CHECK-DAG: [[gBindEntry]] = !{!"pallas.givenBinding", [[gBELoc:![0-9]+]], ptr {{[^,]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, [[fooGDef]]}
// CHECK-DAG: [[gBELoc]] = !{!"pallas.srcLoc", i64 21, i64 23, i64 21, i64 28, !{{[0-9]+}}}


// COM: Yields-binding is encoded correctly
// CHECK-DAG: [[yBindMD]] = !{[[yBindLoc:![0-9]+]], [[yBindEntry:![0-9]+]]}
// CHECK-DAG: [[yBindLoc]] = !{!"pallas.srcLoc", i64 21, i64 34, i64 21, i64 52, !{{[0-9]+}}}
// CHECK-DAG: [[yBindEntry]] = !{!"pallas.yieldsBinding", [[yBELoc:![0-9]+]], [[runYDef]], [[fooYDef]]}
// CHECK-DAG: [[yBELoc]] = !{!"pallas.srcLoc", i64 21, i64 45, i64 21, i64 50, !{{[0-9]+}}}
