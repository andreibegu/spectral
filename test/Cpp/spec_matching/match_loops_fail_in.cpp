// Test that loop invariants are not matched to loops if they are located 
// in the loop
// RUN: not-15 %Spec2IR %s -lib %CppSpecLib -m -lang Cpp
int foo (int x) {
    int cnt = 0;

    while (x < cnt) {
    /*@
    loop_invariant x >= 0;
    @*/
        cnt ++;
    }

    return x; 
}