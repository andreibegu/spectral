// clang-format off
// Test that external contracts get matched correctly.
// RUN: rm -rf %t && %split-file %s %t && cd %t

//--- c_ext_contr.h

/*@
contract for external foo([0-9]?);
args int i, int j;
returns int;
requires i > 0 && j > 0;
ensures _result(int) > 0;
@*/


//--- c_ext_contr.c

/*@
declare DEF_RESULT(int);
@*/


int foo(int i, int j);

int foo42(int i, int j);

int foo0(int i, int j);

int bar(int i, int j);

int foobar(int i, int j) {
    return foo(i, j) + foo0(i, j) + bar(i, j) + foo42(i, j);
}

/*@ wrapper location ; @*/

// RUN: %Spec2IR c_ext_contr.c -lib %CSpecLib -m -cLib c_ext_contr.h | %FileCheck c_ext_contr.c
// CHECK: Matched external contract at 2-8 to foo
// CHECK-DAG: Matched external contract at 2-8 to foo0