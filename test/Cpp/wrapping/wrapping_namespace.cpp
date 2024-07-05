// Test that namespaces are correctly handled when generating the 
// wrapper-functions. 
// The following should not cause an error
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp

#include <vector>
/*@
declare #include <vector> //;
@*/

/*@
requires v.size() > 0;
@*/
int bar(std::vector<int> v)
{
    return v.size();
}