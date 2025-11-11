// clang-format off
// Test that an error is raised if multiple contracts are matched to the same 
// llvm element 
// RUN: rm -rf %t && %split-file %s %t && cd %t

//--- mult_f_contr.c
// RUN: %not %Spec2IR mult_f_contr.c -lib %CSpecLib -m -lang C
/*@
requires a > 0; 
@*/
/*@
requires b > 0;
@*/
int funkyFunction(int a, int b) {
    return 2 * b + 7 * a;
}

//--- mult_loop_inv.c
// RUN: %not %Spec2IR mult_loop_inv.c -lib %CSpecLib -m -lang C

int foo (int x) {
    int cnt = 0;

    /*@
    loop_invariant 0 <= cnt; 
    @*/
    /*@
    loop_invariant cnt <= 42; 
    @*/
    while (cnt < 42) {
        cnt = (cnt + 1) * cnt;
    } 

    return x + cnt; 
}


//--- mult_spec_stmnt.c
// RUN: %not %Spec2IR mult_spec_stmnt.c -lib %CSpecLib -m -lang C

int funkyFunction(int a, int b) {

    /*@
    assert a > 0;
    @*/
    /*@
    assert b > 0;
    @*/
    return 2 * b + 7 * a;
}

