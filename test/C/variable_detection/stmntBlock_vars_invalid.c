// clang-format off
// Check that only valid variables can be referenced from 
// specification-statement-blocks.
// RUN: rm -rf %t && split-file-15 %s %t && cd %t

//--- vars_invalid_1.c
// RUN: not-15 %Spec2IR vars_invalid_1.c -lib %CSpecLib -lang C

int foo (int x, int y) {
    /*@ assert cnt >= 0; @*/
    int cnt = 0;
    float v = 1.0;
    return x; 
}

//--- vars_invalid_2.c
// RUN: not-15 %Spec2IR vars_invalid_2.c -lib %CSpecLib -lang C

int bar (int a, double b) {
    if (a > 5) {
        /*@ assert cnt >= 0; @*/
        return a;
    } else {
        int cnt = 0;
        while (cnt < 10) {
            b += cnt;
        } 
        return b;
    }
}

//--- vars_invalid_3.c
// RUN: not-15 %Spec2IR vars_invalid_3.c -lib %CSpecLib -lang C

int bar (int a, double b) {
    if (a > 5) {
        return a;
    } else {
        int cnt = 0;
        while (cnt < 10) {
            b += cnt;
        } 
        return b;
    }
    /*@ assume cnt >= 0; @*/
}

//--- vars_invalid_4.c
// RUN: not-15 %Spec2IR vars_invalid_4.c -lib %CSpecLib -lang C

int bar (int a, double b) {
    if (a > 5) {
        return a;
    } else {
        int cnt = 0;
        while (cnt < 10) {
            int tmp = cnt + 1;
            b += tmp;
        } 
        /*@ assume tmp >= 0; @*/
        return b;
    }
}

//--- vars_invalid_5.c
// RUN: not-15 %Spec2IR vars_invalid_5.c -lib %CSpecLib -lang C

int foo (int x, int y) {
    int cnt = 0;
    float v = 1.0;
    /*@ assert tmp >= 0; @*/
    int tmp = cnt + 5;
    return x; 
}


