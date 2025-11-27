// Test that the result-construct is transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] double @"pallas.result double"()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: = call double @"pallas.result double"()


/*@
 declare DEF_RESULT(double) ;
@*/

/*@
ensures _result(double) >= 0;
@*/
double fun (double a) {
    return a > 0 ? a : -1 * a;
}