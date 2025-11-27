// clang-format off
// Expected to pass, only valid variables are referenced.
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp

class CoolClass {
    int foo(int x, int y);
};

int foo (int x, int y) {
    /*@
    assert x >= 0 && y >= 0;
    @*/
    int cnt = 0;
    float v = 1.0;
    /*@
    assert x >= 0 && y >= 0 && cnt >= 0 && v >= 0;
    @*/
    return x; 
}

// Test with non-linear control-flow 
int bar (int a, double b) {
    /*@ assert a > 0 && b > 0; @*/
    if (a > 5) {
        /*@ assume a > 0 && b > 0; @*/
        return a;
    } else {
        /*@ assume a > 0 && b > 10; @*/
        int cnt = 0;
        /*@ assert a > 0 && b > 10 && cnt > 0; @*/   
        while (cnt < 10) {
            /*@ assert a > 1 && b > 2 && cnt > 3; @*/   
            b += cnt;
            /*@ assert a > 4 && b > 5 && cnt > 6; @*/   
            b *= 2;
        }
        /*@ assume a > 0 && b > 10 && cnt > 0; @*/   
        return b;
    }
}