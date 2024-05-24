// Example for a simple function contract
// Compile to IR with clang-17 -emit-llvm -S -g -O0 spec_block.c -o spec_block.ll
// Or for var-to-reg promotion with clang-17 -emit-llvm -S -g -O0 spec_block.c -o spec_block.ll -Xclang -disable-O0-optnone 
// and opt-15 -passes='mem2reg' examples/spec_block.ll -o examples/spec_block_reg.ll -S	

/*@
 requires a >= b ;
 ensures 0 >= a && a >= b;
 @*/
// ensures \result >= a && \result >= b;
int bar (int a, int b) {
    int c = 5;
    c += 2;
    a += 5;
    return a + b + c ;
}

