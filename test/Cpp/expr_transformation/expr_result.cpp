// Test that the result-construct is transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] double [[res:@pallas.result.[0-9]+]]()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: = call double [[res]]()

/*@
ensures pallasSpec::result<double>() >= 0;
@*/
double fun (double a) {
    return a > 0 ? a : -1 * a;
}