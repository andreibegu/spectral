// clang-format off
// Test that definitions of ghost arguments are wellformed.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

/*@
given g: Int;
yields y: Int;
requires g > 0;
ensures y > 0;
@*/
func foo (_ x: Int) -> Int {
    return x; 
}


// COM: Get names of function
// CHECK-DAG:  = distinct !DISubprogram(name: "foo", linkageName: [[fooLName:"[^"]+"]], 

// COM: Contract is attached to function 
// CHECK-DAG: define hidden swiftcc i64 @[[fooLName]]({{.*}} !pallas.fcontract [[contractMD:![0-9]+]]

// CHECK-DAG: [[contractMD]] = !{{{.*}}, i1 false, i1 false, [[gArgs:![0-9]+]], [[yArgs:![0-9]+]], [[req:![0-9]+]], [[ens:![0-9]+]]}

// COM: Ghost arguments are correctly defined in the contract
// CHECK-DAG: [[gArgs]] = !{[[gDef:![0-9]+]]}
// CHECK-DAG: [[yArgs]] = !{[[yDef:![0-9]+]]}
// CHECK-DAG: [[gDef]] = !{[[gLoc:![0-9]+]], !"g"}
// CHECK-DAG: [[yDef]] = !{[[yLoc:![0-9]+]], !"y"}

// COM: Source-locations of arg-defs
// CHECK-DAG: [[gLoc]] = !{!"pallas.srcLoc", i64 6, i64 1, i64 6, i64 13, {{.*}}}
// CHECK-DAG: [[yLoc]] = !{!"pallas.srcLoc", i64 7, i64 1, i64 7, i64 14, {{.*}}}

// COM: Args passed to requires-clause
// CHECK-DAG: [[req]] = !{!"pallas.requires", {{.*}}, ptr @{{[^,]*}}, [[reqGiven:![0-9]+]], [[reqYields:![0-9]+]], {{![0-9]+}}}
// CHECK-DAG: [[reqGiven]] = !{[[reqGM1:![0-9]+]]}
// CHECK-DAG: [[reqGM1]] = !{[[gDef]], [[rWG:![0-9]+]]}
// CHECK-DAG: [[reqYields]] = !{}
// CHECK-DAG: [[rWG]] = !DILocalVariable(name: "g", arg: 2, scope: [[rWSP:![0-9]+]]
// CHECK-DAG: [[rWSP]] = distinct !DISubprogram(name: "PALLAS_SPEC_

// COM: Args passed to ensures-clause
// CHECK-DAG: [[ens]] = !{!"pallas.ensures", {{.*}}, ptr @[[ensW:[^,]+]], [[ensGiven:![0-9]+]], [[ensYields:![0-9]+]], {{![0-9]+}}}
// CHECK-DAG: [[ensGiven]] = !{[[ensGM1:![0-9]+]]}
// CHECK-DAG: [[ensYields]] = !{[[ensYM1:![0-9]+]]}
// CHECK-DAG: [[ensGM1]] = !{[[gDef]], [[eWG:![0-9]+]]}
// CHECK-DAG: [[ensYM1]] = !{[[yDef]], [[eWY:![0-9]+]]}
// CHECK-DAG: [[eWG]] = !DILocalVariable(name: "g", arg: 2, scope: [[eWSP:![0-9]+]]
// CHECK-DAG: [[eWY]] = !DILocalVariable(name: "y", arg: 3, scope: [[eWSP]]
// CHECK-DAG: [[eWSP]] = distinct !DISubprogram({{.*}}linkageName: [[ensW]]