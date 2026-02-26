# How to extend the specification-library

This document gives a brief outline on how to add support for an additional specification-construct to the specification-transformation. 

### Extend the unification-stage
The first step in adding support for a new specification-construct is in extending the unification-stage of the core-framework. 

1. Add a case for the new specification-construct to the `SpecLibElement`-enum within `SpecUnifier.h`.
2. Add a factory-method for the unified specification-library. 
This method must return a pointer to the function of the unified specification-library that represents
the new specification-construct. Most of the already implemented factory-methods add a member to the SpecUnifier and
generate the new function when they are first called. As an example, look at `getUSpecSepConj` in `SpecUnifier.cpp`.
3. Add a virtual function to the SpecUnifier that is called to transform the source-language specific version of the specification-construct into the unified representation. The transformation-function is passed a pointer to the call-instruction of the source-language specific specification-function and returns a call to the corresponding specification-construct of the unified specification-library. 
Do not make the function pure-virtual. Instead, provide a default-implementation that just throws an exception
or performs a simple 1:1 translation of the function. 
This way we can support frontends that only support a limited subset of the specification-library. 
As an example, look at `transformSepConj` un `SpecUnifier.h` and `SpecUnifier.cpp`.\
4. Extend `transformSpecCall` to call the new transformation-functions. 
In most cases you will only need to add the new enum-case that you added in 1. to the switch statement and call the 
method that you added in 3.
5. Extend the documentation of the unified specification-library. This is currently located in `res/spec_libs/UnifiedSpecLib.md`.

### Extend the language-specific frontend(s)

1. Extend the specification-library of the frontend. I.e. add a function declaration for the new specification-construct. 
2. Add (a) regression test(s) that cover(s) the transformation of the new specification-construct. 
3. Modify the `isSrcSpecificationFunction`-method of the source-language specific subclass of SpecUnifier so that calls to the frontend-specific specification-library are correctly detected and classified. The correct implementation depends on the frontend. 
4. Override the transform-method for the new specification-construct 
(which you added to the base-implementation of the SpecUnifier in a previous step) 
in the language-specific subclass of the SpecUnifier. 
In this transformation-function, you need to construct a call to the appropriate specification-function of the 
unified specification based on the call to the language-specific specification-function. 
