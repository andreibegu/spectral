// clang-format off
// Test that generated contracts are matched properly 
// RUN: %Spec2IR %s -lib %CppSpecLib -m | %FileCheck %s

/*@
declare using namespace pallasSpec;
@*/

class CheerfulClass {
    public: 
    int i = 0;
    int j = 2; 

    // Contract for the generated constructor
    /*@
    contract for generated src CheerfulClass;
    requires _sep(_Perm(&this->i, _write), _Perm(&this->j, _write)); 
    ensures _sep(_Perm(&this->i, _write), _Perm(&this->j, _write));
    ensures (this->i == 0) && (this->j == 2);
    @*/

};

int foo(int i, int j) {
    CheerfulClass c;
    c.i = 1;
    c.j = 3;
    return 42;
}

// CHECK: Matched generated contract at 15-20 to CheerfulClass
// CHECK-NOT: Matched