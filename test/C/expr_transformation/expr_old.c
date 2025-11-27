// Test that expressions that include the old-construct are transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] float @"pallas.old float_noundef float"(float noundef)
// CHECK-DAG: declare !pallas.specLib [[oldMD]] double @"pallas.old double_noundef double"(double noundef)
// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called
// CHECK-DAG: = call float @"pallas.old float_noundef float"(float 
// CHECK-DAG: = call double @"pallas.old double_noundef double"(double 


/*@
 declare DEF_OLD(double) ;
 declare DEF_OLD(float);
@*/

/*@
ensures _old(double)(a) == a;
ensures _old(float)(b) == b;
@*/
void fun (double a, float b) {
    double c = a + b;
}