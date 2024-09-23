// clang-format off
// Test that a function-contact is not matched to a function that occurs 
// before the contract
// RUN: not-15 %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

func foo(i: Int) {
    let x = i + 1;
}

/*@
 requires i > 0;
 @*/