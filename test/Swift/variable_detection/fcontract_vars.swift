// clang-format off
// Check that the correct variables are detected for use in function contracts.
// This only needs to pass to check that the compilation succeeded. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

// Test for parameters with argument labels
/*@
requires a > 0 && b == true && c > 0;
@*/
func fabulousFunction(pA a: Int, pB b: Bool, pC c: Double) -> Int {
    return b ? a + Int(c) : Int(c);
}

// Test for parameter with _ as argument label
/*@
requires a > 0;
@*/
func funkyFunction(_ a: Int) -> Int {
    return a + 42;
}

// Test for a parameter with a default value
/*@
requires a > 0;
ensures a > 0;
@*/
func fearlessFunction(a: Int = 5) -> Int {
    return a + 5;
}

// Test for an inout-parameter
/*@
requires p1 > 0 && p2 > 0;
ensures p1 > 0;
@*/
func fascinatingFunction(p1: inout Double, p2: Double) {
    p1 += p2;
}
