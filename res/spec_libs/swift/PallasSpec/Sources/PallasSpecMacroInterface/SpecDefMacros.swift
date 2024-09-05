

// Declarations of macros that are used to generate functions for the specification-library
@freestanding(declaration)
public macro defOld<T>() = #externalMacro(module: "PallasSpecMacroImpl", type: "OldDefMacro")