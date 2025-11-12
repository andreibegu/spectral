// Check that scoping of variables works correctly. 
// I.e. multiple variables with the same name are fine, as long as they
// are in different scopes.
// Expected to pass
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp

int foo (int x) {
    int tmp = 0;
    
    {
        int i = 42;
        tmp += x * i;
        i ++;
        /*@
        assert i == 43;
        @*/
        tmp -= i;
    }

    tmp --;

    {
        int i = 9;
        tmp *= i;
        i += 6;
        /*@
        assert i == 15;
        @*/
        tmp += i;
    }

    return x; 
}
