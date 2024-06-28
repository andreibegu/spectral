// Test that the result-construct is transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | FileCheck-15 %s
// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] double [[res:@pallas.result.[0-9]+]]()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: = call double [[res]]()


/*@
 declare DEF_RESULT(double) ;
@*/

/*@
ensures RESULT(double)() >= 0;
@*/
double fun (double a) {
    return a > 0 ? a : -1 * a;
}