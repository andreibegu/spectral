// clang-format off
// Test that a function-contact is correctly detected and matched.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

func foo(i: Int) -> Int {
    return i + 1;
}

/*@
 requires a >= b ;
 ensures 0 >= a && a >= b;
 @*/
func bar (a: Int, b: Int) -> Int {
    var c = 5;
    c += 2;
    return a + b + c ;
}



// COM: Check that contract has been matched correctly
// CHECK: Matched contract at 9-12 to bar
// CHECK-NOT: Matched contract

// COM: Check that contract is attached to the correct function
// CHECK-DAG: [[barMD:![0-9]+]] = distinct !DISubprogram(name: "bar"
// CHECK-DAG: !dbg [[barMD]] !pallas.fcontract