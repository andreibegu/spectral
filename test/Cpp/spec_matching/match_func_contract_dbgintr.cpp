// Test the case where a debug-intrinsic is directly attached to a function-
// argument. This casues the intrinsic to be on the same line as the function. 
// The matching should still succeed.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s
#include <vector>

/*@
ensures pallasSpec::result<int>() >= 0;
@*/
int bar(std::vector<int> v)
{
    int sum = 0;
    for (auto e : v) {
        sum += e;
    }
    return sum;
}

// COM: Check that contract has been matched correctly
// CHECK: Matched contract at 7-9 to bar