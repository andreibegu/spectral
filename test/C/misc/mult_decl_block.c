// clang-format off
// Test that multiple declares-blocks can be used.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s
#include <stdbool.h>

/*@
declare DEF_RESULT(bool);
@*/

/*@
ensures _result(bool) == !a;
@*/
bool foo(bool a)
{
    return !a;
}

typedef long long FancyTypeAlias;

/*@
declare DEF_RESULT(FancyTypeAlias);
@*/

/*@
ensures _result(FancyTypeAlias) == 0;
@*/
FancyTypeAlias bar() {
    return 0;
}

// CHECK-DAG: Matched contract at 10-12 to foo
// CHECK-DAG: Matched contract at 24-26 to bar