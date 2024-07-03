// Test that a function-contact is correctly detected and matched.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

void foo(int i) {
    i++;
}

/*@
 requires a >= b ;
 ensures 0 >= a && a >= b;
 @*/
int bar (int a, int b) {
    int c = 5;
    c += 2;
    a += 5;
    return a + b + c ;
}

// Normal comments should be ignored
// ensures 1 > 2;
float foobar(int x, float y) {
    return x + y;
}


// COM: Check that contract has been matched correctly
// CHECK: Matched contract at 8-11 to bar
// CHECK-NOT: Matched contract

// COM: Check that contract is attached to the correct function
// CHECK-DAG: [[barMD:![0-9]+]] = distinct !DISubprogram(name: "bar"
// CHECK-DAG: !dbg [[barMD]] !pallas.fcontract