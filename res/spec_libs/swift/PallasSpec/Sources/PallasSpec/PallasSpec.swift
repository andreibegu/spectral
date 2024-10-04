/// Logical implication (i.e. b1 ==> b2)
infix operator ==>: ImplicationPrecedence

precedencegroup ImplicationPrecedence {
    higherThan: 
        TernaryPrecedence, 
        AssignmentPrecedence
    lowerThan: 
        BitwiseShiftPrecedence, 
        MultiplicationPrecedence, 
        AdditionPrecedence, 
        RangeFormationPrecedence, 
        CastingPrecedence, 
        NilCoalescingPrecedence, 
        ComparisonPrecedence, 
        LogicalConjunctionPrecedence, 
        LogicalDisjunctionPrecedence 
    associativity: left
    assignment: false
}

public extension Bool {
    @inline(never)
    static func ==> (left: Bool, right: Bool) -> Bool {
        fatalError();
    } 
}

/// Separating conjunction (i.e. b1 ** b2)
@inline(never)
public func _sep(_ b1: Bool, _ b2: Bool) -> Bool {
    return b1 && b2 ;
}

/// Universal quantifier
@inline(never)
public func _forall(_ range: Bool, _ expr: Bool) -> Bool { fatalError() }

/// Separating universal quantifier
@inline(never)
public func _forallSep(_ range: Bool, _ expr: Bool) -> Bool { fatalError() }

/// Existential quantifier
@inline(never)
public func _exists(_ range: Bool, _ expr: Bool) -> Bool { fatalError() }

// Placeholder-struct that represents fractions
public struct Frac {

    // Placeholders, such that the struct is not split into its indvidual fields
    public var placeholder1: Int64 = 0
    public var placeholder2: Int64 = 0
    public var placeholder3: Int64 = 0
    public var placeholder4: Int64 = 0
    public var placeholder5: Int64 = 0

    /// Constructor for creating a new fraction
    public init(_ numerator: Int32, _ denominator: Int32) {
        placeholder1 = Int64(numerator)
        placeholder2 = Int64(denominator)
    }
}