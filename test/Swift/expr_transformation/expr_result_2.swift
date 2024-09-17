// clang-format off
// Test that the result-construct works with different return--types. 
// Only check if the transformation returns withput an error
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift 


/*@
ensures _result() == (p1 > p2);
@*/
func returnsBool(_ p1: Double, _ p2: Double) -> Bool {
    let b: Bool = p1 > p2;
    return b;
}

/*@
ensures _result() != 0;
@*/
func returnsInt(_ p1: Double, _ p2: Double) -> Int {
    let i: Int = Int(p1 + p2);
    return i;
}

/*@
ensures _result() != 0.0;
@*/
func returnsFloat(_ p1: Double, _ p2: Double) -> Float {
    let f: Float = Float(p1 + p2);
    return f;
}
