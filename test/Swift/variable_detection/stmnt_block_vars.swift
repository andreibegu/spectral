// clang-format off
// Check that the correct variables are detected for use in statement-blocks.
// This only needs to pass to check that the compilation succeeded. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

func scaryStatements(x: Double, y:Double) -> Double {
    /*@ 
    assume x > y; 
    @*/
    let d1 = x * y
    let d2 = d1 + 10
    /*@ 
    assert x > y && d1 > d2; 
    @*/
    var d3 = 0.0

    if d2 > 0 {
        /*@ 
        assert d3 == 0 && x + y + d1 + d2 > 0 ;
        @*/
        d3 = d2
    } else {
        /*@ 
        assert d3 == 0 && x + y + d1 + d2 > 0  ;
        @*/
        d3 = -d2
    }

    return d3
}


func smartStatements(x: Int) -> Bool {
    var i = 0;
    while i < 42 {
        if i % 2 == 0 {
            let inc = 2;
            /*@ 
            assume x > 0;
            assert x + i + inc > 0;  
            @*/
            i += inc
        } else {
            i += 1
        }
    }
    /*@ 
    assert x + i > 0;  
    @*/
    return true
}