// clang-format off
// Test that external contracts are matched correctly.
// RUN: rm -rf %t && %split-file %s %t && cd %t

//--- cpp_ext_contr.h

/*@
contract for external (.*)fancy(.+);
args bool i, bool j;
returns bool;
ensures _result<int>() == (i || j) && !(i && j);
@*/


//--- cpp_ext_contr.cpp

/*@
declare using namespace pallasSpec;
@*/

bool super_fancy_function(bool i, bool j);

int bar(int i);

bool fancyFunction(bool a, bool b);

bool fanciestFunction(bool b1, bool b2);

int foobar(bool a, bool b) {
    return super_fancy_function(a, b) && 
           bar(1) > 0 && 
           fancyFunction(a, b) && 
           fanciestFunction(a, b);
}

/*@ wrapper location ; @*/

// RUN: %Spec2IR cpp_ext_contr.cpp -lib %CppSpecLib -m -cLib cpp_ext_contr.h | %FileCheck cpp_ext_contr.cpp
// CHECK: Matched external contract at 2-7 to _Z20super_fancy_functionbb
// CHECK-DAG: Matched external contract at 2-7 to _Z13fancyFunctionbb