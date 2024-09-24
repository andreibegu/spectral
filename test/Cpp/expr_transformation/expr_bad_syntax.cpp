// Check that syntax-errors in expressions trigger an error. 
// RUN: %not %Spec2IR %s -lib %CppSpecLib -m -lang Cpp

/*@
 requires a >= ;
 @*/
int bar (int a, float b) {
    return a + b;
}

