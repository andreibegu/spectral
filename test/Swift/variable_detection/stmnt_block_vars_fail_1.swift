// clang-format off
// Check that the the compilations when variables that are not in scope are 
// are referenced from within a specification-statement. 
// RUN: not-15 %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

func scaryStatements(x: Double, y:Double) -> Double {
    /*@ 
    assume d1 == 0; 
    @*/
    let d1 = x * y
    let d2 = d1 + 10
    var d3 = 0.0

    if d2 + d1 > 0 {
        d3 = d2
    } else {
        d3 = -d2
    }

    return d3
}
