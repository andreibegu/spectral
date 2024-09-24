// clang-format off
// Test expressions that use typealiased-types can be translated. 
// The typealias should be substituted with its definition.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// Check the argument-types in the type of the wrapper-function refer to the 
// un-aliased type.

// CHECK-DAG: !{{[0-9]+}} = distinct !DISubprogram(name: "PALLAS_SPEC_0", {{.*}}type: [[wrapperType:![0-9]+]]
// COM: CHECK-SAME: type: [[wrapperType:![0-9]+]]
// CHECK-DAG: [[wrapperType]] = !DISubroutineType(types: [[typeTuple:![0-9]+]])
// CHECK-DAG: [[typeTuple]] = !{{{![0-9]+}}, [[tdefType:![0-9]+]], [[tdefType]]}
// CHECK-DAG: [[tdefType]] = !DICompositeType(tag: DW_TAG_structure_type, name: "UInt64"


typealias BigIntT = UInt64

/*@
 requires a > 0 && b > 0;
 @*/
func bar (a: BigIntT, b: BigIntT) -> BigIntT {
    return a + b
}
