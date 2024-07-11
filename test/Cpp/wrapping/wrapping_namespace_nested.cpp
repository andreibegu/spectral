// Test that nested namespaces are correctly handled when generating the
// wrapper-functions.
// The following should not cause an error
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp

namespace niceNamespace {

    namespace nestedNamespace {

        struct SomeStruct {
            int a = 0;
        };

        bool funFunction(SomeStruct s) {
            return s.a == 0;
        }
    }

}

/*@
requires niceNamespace::nestedNamespace::funFunction(s) == b;
@*/
int bar(niceNamespace::nestedNamespace::SomeStruct s, bool b)
{
    return s.a;
}