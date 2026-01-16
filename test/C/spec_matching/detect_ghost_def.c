// clang-format off
// Test that definitions of ghost functions are correctly detected
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

/*@
declare DEF_RESULT(int);
@*/


/*@
ghost method
requires i > 0 && j > 0;
ensures  _result(int) > 0;
int foo(int i, int j) {
    return 6 * i + 3 * j;
}
@*/

/*@
ghost function
int bar(int a, int b) { return a + b; }
@*/

// CHECK: Found ghost method foo at 10::1 - 17::1
// CHECK: Found ghost function bar at 19::1 - 22::1