// clang-format off
// Test that loop invariants are correctly matched to (nested) loops.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
#include <stdbool.h>

/*@
declare DEF_RESULT(int);
@*/

/*@
predicate pZero(int *p) := _Perm(p, _write) 
                            && *p == 0;
predicate pOne(int *p) := _Perm(p, _write) && *p == 1;
@*/

/*@
requires pZero(a);
ensures _result(int) == 42;
@*/
int foo (int *a) {
    /*@
    unfold pZero(a);
    assert *a == 0;
    fold pZero(a);
    @*/
    return 42;
}
// CHECK: Found predicate at 11::1 - 12::39
// CHECK: Found predicate at 13::1 - 13::54