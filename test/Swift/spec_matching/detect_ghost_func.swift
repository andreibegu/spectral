// clang-format off
// Test that definitions of ghost functions are correctly detected. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

/*@
ghost function 
requires i > 0 && j > 0;
func aGhost(_ i: Int, _ j: Int) -> Bool {
    return 5 * i > 7 * j
}
@*/


// CHECK: Found ghost function aGhost at 5::1 - 11::1