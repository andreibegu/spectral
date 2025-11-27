// Test that the result-construct is transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] noundef double @"pallas.result noundef double"()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: = call noundef double @"pallas.result noundef double"()

/*@
ensures pallasSpec::_result<double>() >= 0;
@*/
double fun (double a) {
    return a > 0 ? a : -1 * a;
}