// Test that a function-contact is not matched to a function if it is not placed 
// correctly in front of it. 
// In this case the program should exit with an error.
// RUN: not-15 %Spec2IR %s -lib %CppSpecLib -m -lang Cpp

void foo(int i) {
    i++;
}

/*@
 requires i > 0;
 @*/