/*@
requires imply(x > 5, y > 10);
ensures sep(x > 5, y > 10);
@*/
func foo(x: Int, y: Int) -> Int {
    return x + y * (y + 6)
}