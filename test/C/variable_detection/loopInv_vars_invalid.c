// Tests that check that the correct variables are available within a 
// loop-invariant.
// RUN: rm -rf %t && %split-file %s %t && cd %t

//--- vars_invalid_in.c
// Execution is expected to fail because a variable from within the 
// loop is accessed. 
// RUN: %not %Spec2IR vars_invalid_in.c -lib %CSpecLib -m 

int bar (int x) {
    int cnt = 0;
    /*@
    loop_invariant vInv == 0;
    @*/
    while (cnt < 5) {
        float vInv = 0;
        ++cnt;
    } 
    return x; 
}


//--- vars_invalid_after.c
// Execution is expected to fail because a variable that is declared after the 
// loop is used. 
// RUN: %not %Spec2IR vars_invalid_after.c -lib %CSpecLib -m 

int bar (int x) {
    int cnt = 0;
    /*@
    loop_invariant vInv == 0;
    @*/
    while (cnt < 5) {
        ++cnt;
    } 
    float vInv = 0;
    return x; 
}

//--- vars_invalid_nonex.c
// Execution is expected to fail because a non-existent variable is used.
// RUN: %not %Spec2IR vars_invalid_nonex.c -lib %CSpecLib -m 

int bar (int x) {
    int cnt = 0;
    /*@
    loop_invariant vInv == 0;
    @*/
    while (cnt < 5) {
        ++cnt;
    } 
    float vInv = 0;
    return x; 
}


//--- vars_invalid_nested.c
// Execution is expected to fail because a variable is used that 
// is not in scope.
// RUN: %not %Spec2IR vars_invalid_nonex.c -lib %CSpecLib -m 
int bar (int a) {
    int cnt = 0;
    while (cnt < 10) {
       /*@
       loop_invariant v == 0;
       @*/ 
        for (int i = 5; i > 0; i--) {
            cnt ++;
        }
        int v = 0;
    } 
    return cnt;
} 