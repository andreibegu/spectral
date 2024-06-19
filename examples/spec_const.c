/*@
requires Perm(a, fracOf(1,2)) && Perm(b, fracOf(1,2));
@*/
int foo (const int *a, int * const b, const int c){
    return *a + *b + c;
}