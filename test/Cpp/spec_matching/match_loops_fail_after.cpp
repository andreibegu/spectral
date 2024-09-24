// Test that loop invariants are not matched to loops if they are located 
// after the loop
// RUN: %not %Spec2IR %s -lib %CppSpecLib -m -lang Cpp
int foo (int x) {
    int cnt = 0;

    while (x < cnt) {
        cnt ++;
    }
    /*@
    loop_invariant x >= 0;
    loop_invariant cnt >= 0;
    @*/

    return x; 
}