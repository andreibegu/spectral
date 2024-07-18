// clang-format off
// Test matching of statement-blocks in invalid positions 
// RUN: rm -rf %t && split-file-15 %s %t && cd %t

//--- after_return.cpp
// RUN: not-15 %Spec2IR after_return.cpp -lib %CppSpecLib -m -lang Cpp
int foo (int x) {
    return x; 
    /*@ assume x > 0; @*/
}

//--- outside_func.cpp
// RUN: not-15 %Spec2IR outside_func.cpp -lib %CppSpecLib -m -lang Cpp

/*@ assume x > 0; @*/

int foo (int x) {
    int cnt = 0;
    return x + cnt; 
}