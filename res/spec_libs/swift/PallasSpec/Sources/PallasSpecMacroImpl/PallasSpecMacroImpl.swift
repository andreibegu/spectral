import SwiftCompilerPlugin
import SwiftSyntaxMacros

@main
struct PallasSpecMacroImpl: CompilerPlugin {
    var providingMacros: [Macro.Type] = [OldDefMacro.self]
}