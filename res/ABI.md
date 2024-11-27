# ABI issues

- Different compilers use different ABIs, which causes problems during the unification-stage. I.e. if a small struct is passed to a function of the specification-library, the signature of that function differers form the signature for a large struct etc. 
- E.g. clang represents a small struct of a 32-bit int and float as a 64-bit integer. Swiftc returns this struct as a compund-type and passes it by decomposing it into its individual elements. Rust behaved similar to Swiftc, but e.g. uses different threshold as to when a struct is decomposed. 

# ABI handling in LLVM:
- It seems that the frontend lowers the types of the source-language along with appropriate attributes in such a way, that the code generation produced code that matched the targeted ABI
- This apparently has to be handled by the frontend due to discrepancies in the type-system of the source-language and LLVM. 
- Moreover, the LLVM-backend does not seem to have any direct notion of ABIs
- This appears to primarily concern how parameters are passed to functions and how results are returned by functions. 
- I.e. in the transformation, this primarily causes problems during the unification-stage when small structs are passed/ returned from specification-functions. 

- [Forum-post on ABI-handling/ type lowering in LLVM](https://forums.swift.org/t/getting-a-better-understanding-of-how-the-swift-calling-convention-impacts-the-avr-abi/69397/2)


# Potential solutions:

## Fix the ABI:
- Check if it is possible to force the different compilers to use the C-ABI for x86_64.
- In this case, the behavior should be identical for all compilers so that we can generate versions of the specification-functions that adhere to how e.g. small structs are passed in the C-ABI. 
- The backend would then have to implement the logic for handling the cases where e.g. SmallStructs are decomposed. 
- The requires that all compilers provide a way of forcing the use of the same ABI. 
- **There does not seem to be a 'universal' ABI, so this approach will not work**


## Define unified representation & extend unification stage
- Define a common representation of how parameters/ return values are passed (e.g. structs always on the stack, ...)
- The language specific frontend would have to implement the conversion in the unification stage
- Could e.g. be implemented as a LLVM-Pass that effectively undoes the changes that were introduced to obtain ABI-compliance. 

### Advantages: 
- This would simplify the interface for the backend
- Would help to keep source-language specifics in the frontend. (If source is also modified)

### Disadvantages:
- ABI-compatability would be broken (i.e. this might cause issues if we wanted to provide 'executable' specifications e.g. for supporting runtime verification. )
- More overhead on the unification-stage
- This would require making more changes to the generated IR of the specifications, weakening the argument of consistency between code and specification. 
- This would probaly require changes to the IR of the source-code itself 
    - If we only apply this transformation to the generated specifications, then the backend will most likely have to implement the same logic again, in order to be able to verify source-code that contains these ABI-specifics
    - If we also apply these transformations to the IR generated from the source-code itself, then we no longe have the argument, that we do not modify the source and that we thus verify the reuslt of the compiler. 
- It might not be clear what a suitable representation e.g.  for passing small structs is. I.e. a model-checking based backend might have different requirements compared to a backend based on deductive verification. 
- Depending on the used backend, there might be differences as to what are allowed unification-operations (e.g. with regard to memory use, layout, ...). E.g. a bit-precise backend might have different needs when compared to VerCors. 

### TODO:
-  Check if the unified code could still be used for e.g. runtime-monitoring, etc. I.e. check if breaking the ABI-format would cause any errors.  

## Generate more specification functions and handle the ABI-specifics in the backend. 
- We generate a custom function for each of the ABI-specific cases
- E.g. if a small struct is represented as a single integer, we generate a version of the _old-function in which the struct is also represented as a single integer
- The logic for handling these ABI-specifics would then have to be implemented in the backend. 


### Advantages:
- This maintains the advantage that the transformation does not modify the IR of the original source code
- The logic for rewriting the function-types would be available in the backend for use with the specifications and with the program-IR. This would avoid a possible duplication of this logic accross frontend and backend 
- This approach might help to keep the transformation easier to verify. E.g. for a model-checking approach a different style of passing parameters might be suitable than e.g. for a deductive verification approach.

### Disadvantages:
- This would requires potential changes to every backend, if a new source-language is supposed to be supported. As a result, the intermediary-representation would potentially be less universal. At the same time the backend would have to handle these ABI-specifics for the verification anyway if they are not 'normalized' by the frontend. 
- Every backend would have to re-implement a possibly very similar logic for undoing the ABI-specific changes.
- The required rewrites to unify the representations might be quite compiler-specific, adding dependencies on compiler-specifics to the backends that should really be independent of this. 

### TODO:
- Check if the logic for undoing the ABI-specific transformations could be bundled in a single LLVM-pass that could be accessible to backend and frontend. 
- Check if the process of "undoing" the transformations that are done to achieve ABI-compliance can be expressed in a generic and reusable way. This would make it possible to keep compiler-specifics out of the backend. 

## Remarks
- **Do we want to keep the transformation as something that keeps the IR as 'true' to the source as possible, or do we prioritize a unified format.**
- It might be beneficial to bundle the functionality in a library that can be loaded to support a specific ABI. This library could then be reused by different backends.
