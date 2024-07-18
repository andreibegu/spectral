// clang-format off
// Test matching of statement-blocks in invalid positions 
// RUN: rm -rf %t && split-file-15 %s %t && cd %t

//--- after_return.c
// RUN: not-15 %Spec2IR after_loop.c -lib %CSpecLib -m -lang C
int foo (int x) {
    return x; 
    /*@ assume x > 0; @*/
}

//--- outside_func.c
// RUN: not-15 %Spec2IR in_loop.c -lib %CSpecLib -m -lang C

/*@ assume x > 0; @*/

int foo (int x) {
    int cnt = 0;
    return x + cnt; 
}