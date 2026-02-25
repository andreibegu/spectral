# Adding a new frontend
This document provides a brief outline of how to add a new frontend to the framework to support a new source-language. 

To add a frontend for a new language, the following components have to be implemented by creating subclasses of the corresponding components of the core-framework: 
- A *specification-library* that encoded specification-constructs on the level of the new source language.
- A *compiler driver* that calls the languages compiler to produce a LLVM-module from a given source file.
- A *wrapper generator* that generates a wrapper-function for a given expression-string.
- A *specification unifier* that converts calls to the language-specific specification-library into the unified IR-level format that is outlined [here](UnifiedSpecLib.md).
- A *syntax resolver* that is used to adapt the specification-syntax to the used language.

Additionally, your implementation needs to be registered with the framework. 

For many operations, the framework provides default-implementations that should work for a wide range of source-languages. 
If any of these default-implementations do not work for your language, you can customize them by overriding the corresponding functions of the  core-framework in your subclass.  

### Directory layout
Our convention is to create folders that are named after the language that the frontend supports under ```/include``` and ```/lib``` for the sources and under ```/res/spec_libs/``` for the specification library.  
The language-independent aspects of the framework are placed under ```/include/core/``` and ```/lib/core/```.

## Specification library
To encode specification constructs into expression of the source-language, a specification-library is required. 
This library should be placed under ```/res/spec_libs/LANG``` and should provide functions and types that correspond to the IR-level specification constructs that are documented [here](UnifiedSpecLib.md).

When defining the specification-library, we found that it is advisable to only use simple language constructs that are transformed into LLVM-IR in a straightforward way.
Otherwise, the implementation of the unification-step becomes complicated and introduces dependencies on how certain language constructs are lowered to LLVM-IR by the used compiler. 
For example, in the case of Swift we do not use generics in the specification-library. Instead, we manually generate the required functions and use function overloading, as transforming the LLVM-IR that is generated from generic functions is hard to translate to the [IR-level representation](UnifiedSpecLib.md). 

For inspiration, have a look at the existing implementation of specification-libraries, which use some different approches:
- In C we use macros to mimic a template-like specification format
- In C++ we use templated functions
- In Swift, we only provide the specification-functions that are not parametrized by a type in the specification library and generate the other specification-functions as nested-functions during the generation of the wrapper-functions. 

## Compiler Driver
The compiler-driver is a class that invokes the source-language's compiler to generate a LLVM-module. 
To implement a compiler-driver for your frontend, create a subclass of ```include/core/SourceCompilerDriver``` and override the ```compileSource```-method. 

When invoking the compiler, the compiler-drive must ensure that the compiler generates debug-information and does not apply optimizations. 
This is required to ensure that our framework can map specifications to their corresponding LLVM-constructs and to infer which variables are in scope of a given specification. 

When implementing the compiler-driver it is desirable to introduce as few dependencies on the concrete compiler as possible.
In our implementations for C, C++ and Swift we e.g. invoke the compilers as a shell-command and then use LLVM's build-in parser to load the generated LLVM-IR. 
This avoids introducing dependencies on the compiler's internal API. 
The compiler-driver should place any temporary files that it creates in the directory that is provided by the protected field ```workingDir``` of ```SourceCompilerDriver```.

## Wrapper Generator
The wrapper-generator is a component that, given a string that represents an expression from the source-language and a set of variables that can be referenced with in the expression, creates a function that returns the value of this expression when evaluated on the variables that are provided as arguments. 

E.g. given the C-expression ```a > 0 && b > 0``` and the variables ```{int a, int b}```, the wrapper-generator would return the following wrapper-function: 
```
bool expr_1(int a, int b) {
    return a > 0 && b > 0;
}
```

To implement a wrapper-generator, create a subclass of ```include/core/SourceWrapperGenerator``` and implement virtual methods. 

## Specification Unifier
The specification-unifier turns calls to the language specific specification-library into the language independent [IR-level representation](UnifiedSpecLib.md).

To implement a new specification-unifier, create a subclass of ```include/core/SpecUnifier``` and implement the required pure-virtual methods. 
For every specification-construct ```X```, the specification-unifier provides a virtual ```transformX```-method.
If the specification-constructs maps directly from your specification-library to its LLVM-representation (i.e. if the function-signature matches), you can use the default implementation.
Otherwise, you have to provide a custom implementation of the method.

## Syntax Resolver
The syntax resolver is used to adapt the specification-syntax to the used source language.
The default-implementation assumes a C-style syntax for function signatures. 

## Registering your Frontend in the Framework
To integrate your frontend into the framework, create a subclass of ```/include/core/SourceSpecTransformer``` and override the pure-virtual methods. 
Next, go to ```/tools/PallasSpec2IR/PallasSpec2IR.cpp``` and integrate your new implementations there. 
Concretely, this requires:
- Adding your language to the ```ClSourceLang```-enum
- Adding your language to the commandline-options
- Adding a case to the ```main```-method to invoke your subclass of the ```SourceSpecTransformer```