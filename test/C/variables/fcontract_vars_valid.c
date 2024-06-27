// Expected to pass, only valid variables are referenced.
// RUN: %Spec2IR %s -lib %CSpecLib -m 

/*@
requires x >= 0 && y > 0;
ensures x >= 0 && y > 0;
@*/
int foo (int x, int y) {
    int cnt = 0;
    float v = 1.0;
    return x; 
}