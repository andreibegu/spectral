// clang-format off
// Test that generated contracts get matched correctly.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

/*@
declare DEF_RESULT(int);
@*/

// Match to all functions that end on 'bar'
/*@
contract for generated src .*bar$;
requires a > 0 && b > 0;
@*/

double bar(double a, double b) {
    return a / b;
}

int foo(int i, int j) {
    return i + (i * j);
}

int foobar(int a, int b) {
    return a + b;
}

// CHECK: Matched generated contract at 10-13 to bar
// CHECK-DAG: Matched generated contract at 10-13 to foobar
// CHECK-NOT: Matched generated contract at 10-13 to foo