// Example for a simple function contract
// Compile to IR with clang-17 -emit-llvm -S -g -O0 spec_block.c -o spec_block.ll
// Or for var-to-reg promotion with clang-17 -emit-llvm -S -g -O0 spec_block.c -o spec_block.ll -Xclang-disable-O0-optnone 
// and TODO

/*
    Some normal comment if front of a function
*/
int foo () {
    return 7;
}

/*@
 requires a >= b ;
 ensures \result >= a && \result >= b;
 @*/
int bar (int a, int b) {
    return a + b ;
}

