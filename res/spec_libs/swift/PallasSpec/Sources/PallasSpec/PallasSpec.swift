/// Logical implication (i.e. b1 ==> b2)
@inline(never)
public func imply(_ b1: Bool, _ b2: Bool) -> Bool {
    return (!b1) || b2 ;
}

/// Separating conjunction (i.e. b1 ** b2)
@inline(never)
public func sep(_ b1: Bool, _ b2: Bool) -> Bool {
    return b1 && b2 ;
}

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