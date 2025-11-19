// Test that predicate definitions are properly detected. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

/*@
declare using namespace pallasSpec;
@*/

/*@
predicate arr(int *a, int n) :=  _forall(_inRange(0, _bv<int>("i"), n), 
                                         _Perm(&a[_bv<int>("i")], _write));
@*/

// CHECK: Found predicate at 9::1 - 10::75