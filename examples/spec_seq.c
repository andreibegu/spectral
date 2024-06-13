/*@
 declare DEF_SEQ(int);
 declare DEF_SEQ(float);
 @*/


/*@
 requires SEQ_SIZE(int)(SEQ_NEW(int)()) == 0;
 ensures SEQ_AT(int)(SEQ_APPEND(int)(SEQ_NEW(int)(), 42), 0) == 42;
 @*/
int bar (int a, float b) {
    int c = 5;
    c += 2;
    a += 5;
    return a + b + c ;
}

