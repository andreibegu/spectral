// Expected to pass, only valid variables are referenced.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp

bool foo(bool a)
{   
    return !a;
}

/*@
ensures b == false;
@*/
bool bar(bool b)
{
    return b;
}