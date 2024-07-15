// Test that the compilation fails, if an undeclared function is used.
// RUN: not-15 %Spec2IR %s -lib %CSpecLib -m 

/*@
requires abs(a) < 10;
@*/
int foo(int a, int b) {
    return a + b;
}