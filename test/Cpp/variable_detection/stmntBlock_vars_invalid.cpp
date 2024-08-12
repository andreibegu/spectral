// clang-format off
// Check that only valid variables can be referenced from 
// specification-statement-blocks.
// RUN: rm -rf %t && split-file-15 %s %t && cd %t

//--- vars_invalid_1.cpp
// RUN: not-15 %Spec2IR vars_invalid_1.cpp -lib %CppSpecLib -lang Cpp

int foo (int x, int y) {
    /*@ assert cnt >= 0; @*/
    int cnt = 0;
    int v = 1 + cnt;
    return x; 
}

//--- vars_invalid_2.cpp
// RUN: not-15 %Spec2IR vars_invalid_2.cpp -lib %CppSpecLib -lang Cpp

int bar (int a, double b) {
    int tmp = 0;
    if (a > 5) {
        /*@ assert cnt >= 0; @*/
        return a;
    } else {
        int cnt = 0;
        while (cnt < 10) {
            b += cnt;
        } 
        tmp += 1;
    }
    /*@ assert tmp >= 0 && tmp <= 1; @*/
    return b;
}

//--- vars_invalid_3.cpp
// RUN: not-15 %Spec2IR vars_invalid_3.cpp -lib %CppSpecLib -lang Cpp

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

//--- vars_invalid_4.cpp
// RUN: not-15 %Spec2IR vars_invalid_4.cpp -lib %CppSpecLib -lang Cpp

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

//--- vars_invalid_5.cpp
// RUN: not-15 %Spec2IR vars_invalid_5.cpp -lib %CppSpecLib -lang Cpp

int foo (int x, int y) {
    int cnt = 0;
    float v = 1.0;
    /*@ assert tmp >= 0; @*/
    int tmp = cnt + 5;
    return x; 
}


