// clang-format off
// Check that the the compilations when variables are referenced that 
// are not in scope. 
// RUN: %not %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

func scaryStatements(x: Double, y:Double) -> Double {
    let d1 = x * y
    let d2 = d1 + 10
    /*@ 
    assume d3 == 0; 
    @*/
    var d3 = 0.0

    if d2 > 0 {
        d3 = d2
    } else {
        d3 = -d2
    }

    return d3
}
