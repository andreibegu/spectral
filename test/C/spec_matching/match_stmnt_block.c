// clang-format off
// Test matching of statement-blocks. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

int bar (int a, int b) {
    /*@
    assume a > 0;
    assume b > 0;
    @*/
    int c = b + 5;
    /*@
    assert c == 5;
    @*/
    c += 2; 
    /*@ assume c == 7; @*/
    a += 5;
    return a + b /*@ assert c > 0; @*/ + c ;
}

// CHECK-DAG: Matched statement-block at 6-9 to instruction at 10::13
// CHECK-DAG: Matched statement-block at 11-13 to instruction at 14::7
// CHECK-DAG: Matched statement-block at 15-15 to instruction at 16::7
// CHECK-DAG: Matched statement-block at 17-17 to instruction at 17::42