// Test that the use of #includes works with the wrapper-generation. 
// Only checks that the compilation does not crash
// RUN: %Spec2IR %s -lib %CSpecLib -m 
#include <stdlib.h>

/*@
requires abs(a) < 10;
@*/
int foo(int a, int b) {
    return a + b;
}