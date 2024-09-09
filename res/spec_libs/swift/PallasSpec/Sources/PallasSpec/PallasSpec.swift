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