// clang-format off
// Check that statement-blocks outside of functions are not matched. 
// RUN: %not %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

func foo(i: Int) {
    let x = i + 1;
}

/*@
 assert i > 0;
 @*/