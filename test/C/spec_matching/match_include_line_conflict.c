// Test that only functions from the actual source-file are considered during 
// the matching-process. 
// In the test functions are defined on the same line in the header- and 
// in the source-file. 
// RUN: rm -rf %t && split-file-15 %s %t && cd %t

//--- line_conflict.h





int bar(int a, int b) {
    return a + b - 4;
}

//--- line_conflict.c
#include "line_conflict.h"

/*@
requires bar(x, 5) >= 0;
@*/
int foo (int x) {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0 && x >= 0;
    @*/
    while (cnt < 5) {
        ++cnt;
    } 
    return bar(x, cnt); 
}


// RUN: %Spec2IR line_conflict.c -lib %CSpecLib -m -lang C | FileCheck-15 %s
// CHECK: Matched contract at 3-5 to foo