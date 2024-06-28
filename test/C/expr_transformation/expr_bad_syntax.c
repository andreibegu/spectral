// Check that syntax-errors in expressions trigger an error. 
// RUN: not-15 %Spec2IR %s -lib %CSpecLib -m

/*@
 requires a >= ;
 @*/
int bar (int a, float b) {
    return a + b;
}

