// TODO: SpecLib goes here

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

/// Value of v before the function was executed. 
@inline(never)
public func old<T>(_ v: T) -> T {
    return v;
}

// Result
// @_optimize(none)
@inline(never)
public func result<T>() -> T {
    // The body does not have to be executed, hence 
    // it is sufficient if it just compiles without errors.
    return unsafeBitCast(0, to: T.self)
}

