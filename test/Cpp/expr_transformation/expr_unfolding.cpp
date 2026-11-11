// Test that the _unfolding construct for predicates is transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: bool
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] noundef zeroext i1 @"pallas.unfolding noundef zeroext i1_noundef zeroext i1"(i1 noundef zeroext, i1 noundef zeroext)
// CHECK-DAG: [[mdNode]] = !{!"pallas.unfolding"}
// CHECK-DAG: {{%[0-9]+}} = call noundef zeroext i1 @"pallas.unfolding noundef zeroext i1_noundef zeroext i1"(

// COM: ABigClass
// CHECK-DAG: declare !pallas.specLib [[mdNode]] void @"pallas.unfolding sret(%class.ABigClass) align 8 void_noundef byval(%class.ABigClass) align 8 ptr"(ptr sret(%class.ABigClass) align 8, i1 noundef zeroext, ptr noundef byval(%class.ABigClass) align 8)
// CHECK-DAG: call void @"pallas.unfolding sret(%class.ABigClass) align 8 void_noundef byval(%class.ABigClass) align 8 ptr"(

/*@
declare using namespace pallasSpec;
@*/

class ABigClass {
    public:
    int r, g, b;
    double d1, d2, d3;
};

/*@
predicate pPtrWrite(int *a) := _sep(a != NULL, _Perm(a, _writePerm));
@*/

/*@
requires pPtrWrite(p);
requires _unfolding<bool>(pPtrWrite(p), 
                          *p == 0);
requires _unfolding(pPtrWrite(p), c).r == 0;
@*/
void flamboyantFunction(int *p, ABigClass c) {
    *p = 42;
}