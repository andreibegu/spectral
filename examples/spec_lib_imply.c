// Example for a simple function contract that includes a call to a function from the C specification library.
// Compile to IR with clang-17 -emit-llvm -S -g -O0 spec_lib_imply.c -o spec_lib_imply.ll -Xclang -disable-O0-optnone 
// and opt-15 -passes='mem2reg' spec_lib_imply.ll -o spec_lib_imply_reg.ll -S	

/*@
 requires imply(a >= b, a >= 0);
 ensures 0 >= a && a >= b;
 @*/
int bar (int a, int b) {
    int c = 5;
    c += 2;
    a += 5;
    return a + b + c ;
}

