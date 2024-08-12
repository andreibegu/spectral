// clang-format off
// Test matching of statement-blocks.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

class CoolClass {
    public:
    int i;
    int j;
};

int bar (int a, CoolClass c) {
    /*@
    assert a >= 0;
    assume c.i == 0;
    @*/
    c.i = a;
    /*@ assert c.i == a; @*/
    a = c.j;
    return c.j + c.i /*@ assert c.i >= 0; @*/ + 5;
}

// CHECK-DAG: Matched statement-block at 12-15 to instruction at 16::11
// CHECK-DAG: Matched statement-block at 17-17 to instruction at 18::11
// CHECK-DAG: Matched statement-block at 19-19 to instruction at 19::47
