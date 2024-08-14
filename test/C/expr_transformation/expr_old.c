// Test that expressions that include the old-construct are transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | FileCheck-15 %s
// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] float [[oldF:@pallas.old.[0-9]+]](ptr)
// CHECK-DAG: declare !pallas.specLib [[oldMD]] double [[oldD:@pallas.old.[0-9]+]](ptr)
// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called
// CHECK-DAG: = call float [[oldF]](ptr 
// CHECK-DAG: = call double [[oldD]](ptr 


/*@
 declare DEF_OLD(double) ;
 declare DEF_OLD(float);
@*/

/*@
ensures OLD(double)(&a) == a;
ensures OLD(float)(&b) == b;
@*/
void fun (double a, float b) {
    double c = a + b;
}