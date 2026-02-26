// clang-format off
// Test that definitions of ghost functions are correctly detected
// RUN: %Spec2IR %s -lib %CppSpecLib -m | %FileCheck %s

/*@
declare using namespace pallasSpec;
@*/

/*@
ghost function
requires i > 0 && j > 0;
ensures  _result<int>() > 0;
int foo(int i, int j) { return 6 * i + 3 * j; }
   @*/


// CHECK: Found ghost function foo at 9::1 - 14::4