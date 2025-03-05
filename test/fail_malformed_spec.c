// clang-format off
// Test the malformed specification-blocks cause an error 
// instead of being ignored.
// RUN: rm -rf %t && %split-file %s %t && cd %t

//--- bad_require.c
// RUN: %not %Spec2IR bad_require.c -lib %CSpecLib -m -lang C

/*@
requires x > 0;
require x >= 0;
@*/
int foo (int x) {
    return x; 
}

//--- bad_invariant.c
// RUN: %not %Spec2IR bad_invariant.c -lib %CSpecLib -m -lang C

int foo (int x) {
    int i = 0;
    /*@
    loop_invariant i <= x;
    loop_invariant i >= 0
    @*/
    while (i < x) {
        i++;
    }
    return x; 
}


//--- bad_assert.c
// RUN: %not %Spec2IR bad_assert.c -lib %CSpecLib -m -lang C

int foo (int x) {
    int i = 0;
    /*@
    assert i == 0;
    ;
    @*/
    
    return x; 
}
