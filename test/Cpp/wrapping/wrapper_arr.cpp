// clang-format off
// Regression test to ensure that wrapper function are sucessfully generated 
// when arrays are passed as arguments. 
// Expected to pass. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m 

/*@
declare using namespace pallasSpec;
@*/

/*@
requires _forall(_inRange(0, _bv<int>("i"), 42), 
                 _Perm(&a0[_bv<int>("i")], _fracOf(1, 2)));
@*/
int bar (int a0[]) {
    int a1[42];
    double a2[42];
    int sum = 0;

    /*@
    loop_invariant _forall(_inRange(0, _bv<int>("i"), 42), 
                           _Perm(&a0[_bv<int>("i")], _fracOf(1, 2)));
    loop_invariant _forall(_inRange(0, _bv<int>("i"), 42), 
                           _Perm(&a1[_bv<int>("i")], _writePerm));
    @*/
    for (int i = 0; i < 42; ++i) {
        a1[i] = a0[i];
        sum += a0[i];
    }

    /*@
    loop_invariant _forall(_inRange(0, _bv<int>("i"), 42), 
                           _Perm(&a0[_bv<int>("i")], _fracOf(1, 2)));
    loop_invariant _forall(_inRange(0, _bv<int>("i"), 42), 
                           _Perm(&a2[_bv<int>("i")], _writePerm));
    @*/
    for (int i = 0; i < 42; ++i) {
        a2[i] = (double) i;
    }

    return sum;
}