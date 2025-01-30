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
 * Moreover the structs are filled with dummy-values to exceed a length 
 * of 64 bytes to ensure that they are passed as a by-value pointer to functions 
 * (ABI-dependent behavior).
 */

typedef struct
{
    int32_t dummy1, dummy2, dummy3, dummy4, dummy5;
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
    int32_t dummy1, dummy2, dummy3, dummy4, dummy5; \
    } SEQ(type)


/*******************************************
 *          Special expressions            *
 *******************************************/

// Macros to define a version of \result for every possible return-type
#define RESULT(type) PALLAS_RESULT___ ## type
#define DEF_RESULT(type) type RESULT(type) ()

// Macros to define a version of \old(...) for every possible parameter-type
#define OLD(type) PALLAS_OLD___ ## type
#define DEF_OLD(type) type OLD(type) (type var)


/*******************************************
 *              Quantifiers                *
 *******************************************/

// Macros to define bound variables. 
#define BV(type) PALLAS_BOUND_VAR___ ## type
#define DEF_BV(type) type BV(type) (const char *id)

bool exists(bool range, bool expr);
bool forall(bool range, bool expr);
bool forallS(bool range, bool expr);


/*******************************************
 *             Permissions                 *
 *******************************************/

// Function to specify permission over a heap-location
bool Perm(void *loc, frac p);

/*******************************************
 *               Operators                 *
 *******************************************/

// Logical implication (b1 ==> b2)
bool imply(bool lhs, bool rhs);

// Short circuting and-operator for use in specificaions.
bool _and(bool lhs, bool rhs);

// Short circuting or-operator for use in specificaions.
bool _or(bool lhs, bool rhs);

// Separating conjunction (b1 ** b2)
bool sep(bool lhs, bool rhs);

// Define a fraction as a division of two integers
// Only valid for positive parameters with 0 < (numerator \ denominator ) <= 1.
frac fracOf(int numerator, int denominator);

/*******************************************
 *          Sequence-operations            *
 *******************************************/

// Expands to the name of the size-function for sequence<type> & 
// defines the size-function for seqence<type>.
#define SEQ_SIZE(type) PALLAS_SEQ_SIZE___ ## type
#define DEF_SEQ_SIZE(type) size_t SEQ_SIZE(type) ( SEQ(type) s )

// Macros to define the at-function (access at index) for sequence<type>.
#define SEQ_AT(type) PALLAS_SEQ_AT___ ## type
#define DEF_SEQ_AT(type) type SEQ_AT(type) ( SEQ(type) s, size_t idx )

// Macros to define prepending a value to a sequence of type sequence<type>.
#define SEQ_PREPEND(type) PALLAS_SEQ_PREPEND___ ## type
#define DEF_SEQ_PREPEND(type) SEQ(type) SEQ_PREPEND(type) ( type e, SEQ(type) s)

// Macros to define sequence-creation
#define SEQ_NEW(type) PALLAS_SEQ_NEW___ ## type
#define DEF_SEQ_NEW(type) SEQ(type) SEQ_NEW(type) ()

// Macro to define sequence-equality
#define SEQ_EQ(type) PALLAS_SEQ_EQ___ ## type
#define DEF_SEQ_EQ(type) bool SEQ_EQ(type) (SEQ(type) s1, SEQ(type) s2)


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
DEF_SEQ_EQ(type);

