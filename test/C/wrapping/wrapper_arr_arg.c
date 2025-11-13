// clang-format off
// Regression test to ensure that wrapper function are sucessfully generated 
// when arrays are passed as arguments. 
// Expected to pass. 
// RUN: %Spec2IR %s -lib %CSpecLib -m 

/*@
declare DEF_BV(int);
@*/

/*@
requires _Perm(c, _write);
@*/
int foo (int a, int b, int c[]) {
    int x[4];
    int y[1][2];

    x[0] = 0;
    x[1] = 1;
    x[2] = 2;
    x[3] = 3;

    /*@
    assert _forall(_inRange(0, _bv(int, i), 4), x[_bv(int, i)] == _bv(int, i));
    @*/
    return 42;
}
