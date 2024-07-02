// Execution is expected to fail because a variable from within the function is 
// accesssed
// RUN: not-15 %Spec2IR %s -lib %CppSpecLib -m -lang Cpp

/*@
requires x >= 0 && cnt >= 1;
@*/
int foo (int x, int y) {
    int cnt = 0;
    float v = 1.0;
    return x; 
}

