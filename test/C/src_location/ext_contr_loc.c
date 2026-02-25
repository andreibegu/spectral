// Test that source-locations are correctly translated when contracts are loaded 
// from external files.  
// clang-format off
// Test that external contracts get encoded correctly.
// RUN: rm -rf %t && %split-file %s %t && cd %t

//--- c_ext_contr_enc.h

/*@
contract for external inc;
args int x;
returns int;
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
// CHECK-DAG: [[contractMD]] = !{[[cLoc:![0-9]+]], i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[cLoc]] = !{!"pallas.srcLoc", i64 2, i64 1, i64 7, i64 1, [[cFile:![0-9]+]]}
// CHECK-DAG: [[ensMD]] = !{!"pallas.ensures", [[ensLoc:![0-9]+]], ptr {{@PALLAS_SPEC_[0-9]+}}, [[emp]], [[emp]], [[emp]]}
// CHECK-DAG: [[ensLoc]] = !{!"pallas.srcLoc", i64 6, i64 1, i64 6, i64 30, [[cFile:![0-9]+]]}
// CHECK-DAG: [[cFile]] = !DIFile(filename: "{{.*}}/c_ext_contr_enc.h"