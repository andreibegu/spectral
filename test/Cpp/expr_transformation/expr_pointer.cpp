// Check that expressions with pointers get transformed
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// CHECK: @_Z13PALLAS_SPEC_0PiPS_(ptr noundef [[p1:%[0-9]+]], ptr noundef [[p2:%[0-9]+]])
// CHECK: [[tmp:%[0-9]+]] = load ptr, ptr [[p2]]
// CHECK: icmp eq ptr [[p1]], [[tmp]]

/*@
 requires a == *b;
 @*/
void bar (int *a, int **b) {
    int i = 0;
}