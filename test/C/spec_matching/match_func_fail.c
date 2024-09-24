// Test that a function-contact is nt matched to a function if it is not placed 
// correctly in front of it. 
// In this case the program should exit with an error.
// RUN: %not %Spec2IR %s -lib %CSpecLib -m

void foo(int i) {
    i++;
}

/*@
 requires i > 0;
 @*/