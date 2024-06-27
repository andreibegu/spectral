// Test that a function-contact is correctly detected and matched.
// RUN: %Spec2IR %s -lib %CSpecLib -m | FileCheck-15 %s

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
// ensures \result >= a && \result >= b;
float foobar(int x, float y) {
    return x + y;
}
// CHECK: Matched contract at 8-11 to bar
// COM: CHECK-NOT: Matched contract

// CHECK: i32 @bar(
// CHECK-SAME: !pallas.fcontract 

// CHECK-DAG: i1 @PALLAS_SPEC_0(i32 noundef %0, i32 noundef %1)
// CHECK-SAME: !pallas.exprWrapper
// CHECK-DAG: i1 @PALLAS_SPEC_1(i32 noundef %0, i32 noundef %1)
// CHECK-SAME: !pallas.exprWrapper

// CHECK-DAG: !"pallas.requires"
// CHECK-SAME: ptr @PALLAS_SPEC_0
// CHECK-DAG: !"pallas.ensures"
// CHECK-SAME: ptr @PALLAS_SPEC_1


