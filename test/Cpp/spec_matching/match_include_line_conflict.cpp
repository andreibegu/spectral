// Test that only functions from the actual source-file are considered during 
// the matching-process. 
// In the test, max is declared on line 6 in the header-file, and foo on line 
// 10 of the cpp-file. This should not cause an error.
// RUN: rm -rf %t && split-file-15 %s %t && cd %t

//--- line_conflict.h





int max(int a, int b) {
    return a > b ? a : b;
}

//--- line_conflict.cpp
#include "line_conflict.h"

/*@
requires x >= 0;
@*/
int foo (int x) {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0 && x >= 0;
    @*/
    while (cnt < 5) {
        ++cnt;
    } 
    return max(x, x); 
}


// RUN: %Spec2IR line_conflict.cpp -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s
// CHECK: Matched contract at 3-5 to foo