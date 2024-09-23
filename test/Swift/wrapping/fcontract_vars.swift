// clang-format off
// Check that functions and variables form the source-file can be 
// referenced from within specifications. 
// As long as this does not throw an error, it should be fine. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift


// A global variable that should be accessible from within the specifications
var gloriousGlobalVar: Int = 1;

// A function that should be accessible from within the specifications
func glamorousGlobalFunc(p: Int) -> Bool {
    return p > 0;
}

/*@
requires glamorousGlobalFunc(p: p1) && gloriousGlobalVar > 0;
@*/
func theMostAmazingFunctionEver(p1: Int, p2: Double) -> Int {
    var x = 42;

    /*@
    loop_invariant glamorousGlobalFunc(p: x);
    loop_invariant gloriousGlobalVar > 0;
    @*/
    while x > 0 {
        x -= p1;
    } 

    /*@
    assert glamorousGlobalFunc(p: x);
    assert gloriousGlobalVar > 0;
    @*/
    x += Int(p2);
    return x;
}