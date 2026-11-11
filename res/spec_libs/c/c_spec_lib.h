#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*******************************************
 *               Datatypes                 *
 *******************************************/

/*
 * The fields in the datatypes are irrelevant. 
 * However, when the types are defined as empty structs, clang optimizes them 
 * away when they are passed as a parameter to a function. Hence the structs 
 * contain dummmy-values to avoid this. 
 * Moreover the structs is filled with a "randomly"-sized array to prevent it
 * from being merged with a user's type.
 */

typedef struct
{
    int32_t dummy[0x66726163];
} frac;

/* Macros for 'generic' ADTs */

// Macro that expands to the type of a sequence containing 
// elements of type 'type'.
#define SEQ(type) PALLAS_SEQ___ ## type

// Macro that expands to the definition of the type of a sequence with content 
// of type 'type'. The first element in the struct corresponds to the 
// content-type of the sequence, the remaining integers are used to bloat-up the 
// size to suppress ABI-specific optimizations when passing the struct.
#define DEF_SEQ_T(type) typedef struct { \
    type contentType; \
    int32_t dummy[0x736571]; \
} SEQ(type)


/*******************************************
 *          Special expressions            *
 *******************************************/

// Macros to define a version of \result for every possible return-type
// In special case where the type contains characters that may not appear in a 
// function name (e.g. *) the DEF_RESULT_ macro can be used with a custom alias. 
// In the normal case, the alias is equal to the type-name.
#define RESULT_NAME(alias) PALLAS_RESULT___ ## alias
#define DEF_RESULT_(type, alias) type RESULT_NAME(alias) ()
#define DEF_RESULT(type) DEF_RESULT_(type, type)
#define _result(type) RESULT_NAME(type)()

// Macros to define a version of \old(...) for every possible parameter-type
#define _old(type) PALLAS_OLD___ ## type
#define DEF_OLD(type) type _old(type) (type var)

// Macro to refer to the old value of a pointer-type variable, as * cannot be 
// passed to the '_old'-macro. 
#define _oldPtr PALLAS_OLD___ptr
void* _oldPtr (void *v);

/*******************************************
 *              Quantifiers                *
 *******************************************/

// Macros to define bound variables. 
#define BV_NAME(type) PALLAS_BOUND_VAR___ ## type
#define DEF_BV(type) type BV_NAME(type) (const char *id)
#define _bv(type, id) BV_NAME(type)(#id)

bool _exists(bool range, bool expr);
bool _forall(bool range, bool expr);
bool _forallS(bool range, bool expr);


/*******************************************
 *         Permissions & Pointers          *
 *******************************************/

// Function to specify permission over a heap-location
bool _Perm(void *loc, frac p);

// Functions for specifying properties about pointer validity
int64_t _ptr_block_length(void *ptr);
int64_t _ptr_block_offset(void *ptr);
int64_t _ptr_length(void *ptr); 


/*******************************************
 *               Operators                 *
 *******************************************/

// Logical implication (b1 ==> b2)
bool _imply(bool lhs, bool rhs);

// Short circuting and-operator for use in specificaions.
bool _and(bool lhs, bool rhs);

// Short circuting or-operator for use in specificaions.
bool _or(bool lhs, bool rhs);

// Separating conjunction (b1 ** b2)
bool _sep(bool lhs, bool rhs);

// Define a fraction as a division of two integers
// Only valid for positive parameters with 0 < (numerator \ denominator ) <= 1.
frac _fracOf(int numerator, int denominator);

/*******************************************
 *              Predicates                 *
 *******************************************/

// Macros to define a version of \unfolding for expressions with different types.
// In special case where the type contains characters that may not appear in a 
// function name (e.g. *) the DEF_UNFOLDING_ macro can be used with a custom alias. 
// In the normal case, the alias is equal to the type-name.
#define UNFOLDING_NAME(alias) PALLAS_UNFOLDING___ ## alias
#define DEF_UNFOLDING_(type, alias) type UNFOLDING_NAME(alias) (bool pred, type in)
#define DEF_UNFOLDING(type) DEF_UNFOLDING_(type, type)
#define _unfolding(type) UNFOLDING_NAME(type)

/*******************************************
 *          Sequence-operations            *
 *******************************************/

// Expands to the name of the size-function for sequence<type> & 
// defines the size-function for seqence<type>.
#define _seqSize(type) PALLAS_SEQ_SIZE___ ## type
#define DEF_SEQ_SIZE(type) size_t _seqSize(type) ( SEQ(type) s )

// Macros to define the at-function (access at index) for sequence<type>.
#define _seqAt(type) PALLAS_SEQ_AT___ ## type
#define DEF_SEQ_AT(type) type _seqAt(type) ( SEQ(type) s, size_t idx )

// Macros to define prepending a value to a sequence of type sequence<type>.
#define _seqPrepend(type) PALLAS_SEQ_PREPEND___ ## type
#define DEF_SEQ_PREPEND(type) SEQ(type) _seqPrepend(type) ( type e, SEQ(type) s)

// Macros to define sequence-creation
#define _seqNew(type) PALLAS_SEQ_NEW___ ## type
#define DEF_SEQ_NEW(type) SEQ(type) _seqNew(type) ()

// Macro to define sequence-equality
#define _seqEq(type) PALLAS_SEQ_EQ___ ## type
#define DEF_SEQ_EQ(type) bool _seqEq(type) (SEQ(type) s1, SEQ(type) s2)

// Sequence-slice
#define _seqSlice(type) PALLAS_SEQ_SLICE___ ## type
#define DEF_SEQ_SLICE(type) SEQ(type) _seqSlice(type) (SEQ(type) s, size_t start, size_t end)

// Sequence-update / Set element at index
#define _seqSet(type) PALLAS_SEQ_SET___ ## type
#define DEF_SEQ_SET(type) SEQ(type) _seqSet(type) (SEQ(type) s, size_t idx, type value)


/*********************************************
 *     Convenience macros for ADTs           *
 *********************************************/

/* 
 * Bundles the defintion of the sequence type with content 'type'
 * with the definitions of the operations on this sequence-type. 
 */
#define DEF_SEQ(type) \
DEF_SEQ_T(type) ; \
DEF_SEQ_SIZE(type) ; \
DEF_SEQ_AT(type) ; \
DEF_SEQ_PREPEND(type); \
DEF_SEQ_NEW(type); \
DEF_SEQ_EQ(type); \
DEF_SEQ_SLICE(type); \
DEF_SEQ_SET(type);

/*******************************************
 *          Shorthand-notations            *
 *******************************************/

// Macro for write-permission
#define _writePerm _fracOf(1, 1)

#define _inRange(lower, v, upper) _and(lower <= v , v < upper)

// _seqHead(int, s) 
#define _seqHead(type, s) _seqAt(type)(s, 0)

// _seqTail(int, s)
#define _seqTail(type, s) _seqSlice(type)(s, 1, _seqSize(int)(s))