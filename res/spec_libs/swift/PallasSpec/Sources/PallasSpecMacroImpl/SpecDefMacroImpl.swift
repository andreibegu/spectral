import SwiftSyntax
import SwiftSyntaxMacros

/// Implementation of the macro that is used to generate the specification-functions 
// for representing the /old-construct.
public struct OldDefMacro: DeclarationMacro {
  public static func expansion(
    of node: some FreestandingMacroExpansionSyntax,
    in context: some MacroExpansionContext
  ) throws -> [DeclSyntax] {
    
    // Get the generic type-argument of the macro
    guard let genericType = node.genericArgumentClause?.arguments.first else {
      throw SpecMacroError.msg("Failed to get generic type for macro.")
    }

    // Generate a name for the function
    // It would potentially be better to use a fixed name!
    let name = context.makeUniqueName("pallasOld")
    
    return [
    """
    func \(name)(v: \(genericType)) -> \(genericType) { 
      return v 
    }
    """
    ]
  }
}

enum SpecMacroError: Error { case msg(String) }

