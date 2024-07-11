// Test that included headers and function from the same file can be referenced
// in the specifications. 
// The following should not cause an error
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp

#include <vector>

bool foo(bool a) {
    return !a;
}

/*@
requires foo(true) == false;
requires v.size() > 0;
@*/
int bar(std::vector<int> v)
{
    return v.size();
}