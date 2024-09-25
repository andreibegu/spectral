// clang-format off
// Check that variables from inside a function cannot be referenced in the contract.
// RUN: %not %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

/*@
requires c > 0;
@*/
func fabulousFunction(pA a: Int, pB b: Int) -> Int {
    let c = a + b + 8
    return a + c
}
