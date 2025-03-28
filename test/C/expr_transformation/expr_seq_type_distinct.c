// Test two sequences with distinct content-types are encoded as different types
// (i.e. they should not be comparable)
// RUN: %not %Spec2IR %s -lib %CSpecLib -m

/*@
 declare DEF_SEQ(double);
 declare DEF_SEQ(float);
 @*/

// Comparisson of different types, this is expected to fail.
/*@
 requires _seqNew(float)() == _seqNew(double)();
 @*/
int bar (int a, float b) {
    return a + b;
}

