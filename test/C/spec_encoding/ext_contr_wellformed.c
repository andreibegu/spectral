// clang-format off
// Test that external contracts get encoded correctly.
// RUN: rm -rf %t && %split-file %s %t && cd %t

//--- c_ext_contr_enc.h

/*@
contract for external inc;
args int x;
returns int;
requires x > 0;
ensures _result(int) == x + 1;
@*/


//--- c_ext_contr_enc.c

/*@
declare DEF_RESULT(int);
@*/


int inc(int x);


int foobar(int i, int j) {
    return inc(i) + inc(j);
}

/*@ wrapper location ; @*/

// RUN: %Spec2IR c_ext_contr_enc.c -lib %CSpecLib -cLib c_ext_contr_enc.h | %FileCheck c_ext_contr_enc.c
// CHECK: declare !pallas.extContract [[contractMD:![0-9]+]] i32 @inc(i32 noundef)
// CHECK-DAG: [[contractMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", {{![0-9]+}}, ptr {{@PALLAS_SPEC_[0-9]+}}, [[emp]], [[emp]], [[emp]]}
// CHECK-DAG: [[ensMD]] = !{!"pallas.ensures", {{![0-9]+}}, ptr {{@PALLAS_SPEC_[0-9]+}}, [[emp]], [[emp]], [[emp]]}