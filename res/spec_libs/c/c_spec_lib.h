#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*******************************************
 *               Datatypes                 *
 *******************************************/

/*
 * The fields in the datatypes are irrelevant. 
 * However, when the types are defined as empty structs, clang optimizes them away when they are 
 * passed as a parameter to a function. Hence the structs contain dummmy-values to avoid this. 
 * Moreover the structs are filled with dummy-values to exceed a length of 64 bytes to ensure that 
 * they are passed as a by-value pointer to functions (ABI-dependent behavior).
 */

typedef struct
{
    int32_t dummy1, dummy2, dummy3, dummy4, dummy5;
} frac;

typedef struct {
    int32_t dummy1, dummy2, dummy3, dummy4, dummy5;
} resource;

/* Macros for 'generic' ADTs */

// Macro that expands to the type of a sequence containing elements of type 'type'
#define SEQ(type) PALLAS_SEQ___ ## type

// Macro that expands to the definition of the type of a sequence with content of type 'type'.
#define DEF_SEQ_T(type) typedef struct { \
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
#define DEF_BV(type) type BV(type) (char *id)

bool exists(bool range, bool expr);
bool forall(bool range, bool expr);
bool forallP(bool range, bool expr);


/*******************************************
 *             Permissions                 *
 *******************************************/

// Function to specify permission over a heap-location
bool Perm(void *loc, frac p);

/*******************************************
 *               Operators                 *
 *******************************************/

/**
 * Logical implication.
 * b1 ==> b2
 */
bool imply(bool b1, bool b2);

/**
 * Magic wand from separation logic. 
 * b1 -* b2 
 */
bool magicWand(bool b1, bool b2);

/**
 * Separating conjunction 
 * b1 ** b2 
 */
bool separatingConjunction(bool b1, bool b2);

/**
 * Arithmetic operators for fractions.
 */
frac fAdd(frac f1, frac f2);
frac fSub(frac f1, frac f2);
frac fMult(frac f1, frac f2);

/**
 * Define a fraction as a division of two integers
 * Only valid for positive parameters with 0 < (numerator \ denominator ) <= 1.
 */
frac fracOf(int numerator, int denominator);


/*******************************************
 *          Sequence-operations            *
 *******************************************/

/**
 * Create an empty sequence
 */
// #define SEQ(type) PALLAS_SEQ_ ## type
// #define DEF_SEQ_T(type) typedef struct { } SEQ(type)

// Expands to the name of the size-function for sequence<type>. & defines the size-function for seqence<type>.
#define SEQ_SIZE(type) PALLAS_SEQ_SIZE___ ## type
#define DEF_SEQ_SIZE(type) size_t SEQ_SIZE(type) ( SEQ(type) s )

// Macros to define the contains-function for sequence<type>.
#define SEQ_IN(type) PALLAS_SEQ_IN___ ## type
#define DEF_SEQ_IN(type) bool SEQ_IN(type) ( SEQ(type) s, type e )

// Macros to define the at-function (access at index) for sequence<type>.
#define SEQ_AT(type) PALLAS_SEQ_AT___ ## type
#define DEF_SEQ_AT(type) type SEQ_AT(type) ( SEQ(type) s, size_t idx )

// Macros to define the changing the element at a given index for sequence<type>.
#define SEQ_SETAT(type) PALLAS_SEQ_SETAT___ ## type
#define DEF_SEQ_SETAT(type) SEQ(type) SEQ_SETAT(type) ( SEQ(type) s, size_t idx, type e )

// Macros to define appending a value to a sequence of type sequence<type>.
#define SEQ_APPEND(type) PALLAS_SEQ_APPEND___ ## type
#define DEF_SEQ_APPEND(type) SEQ(type) SEQ_APPEND(type) ( SEQ(type) s, type e )

// Macros to define concatenating two sequences of type sequence<type>.
#define SEQ_CONCAT(type) PALLAS_SEQ_CONCAT___ ## type
#define DEF_SEQ_CONCAT(type) SEQ(type) SEQ_CONCAT(type) ( SEQ(type) s1, SEQ(type) s2 )

#define SEQ_NEW(type) PALLAS_SEQ_NEW___ ## type
#define DEF_SEQ_NEW(type) SEQ(type) SEQ_NEW(type) ()


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
DEF_SEQ_IN(type) ; \
DEF_SEQ_AT(type) ; \
DEF_SEQ_SETAT(type); \
DEF_SEQ_APPEND(type); \
DEF_SEQ_CONCAT(type); \
DEF_SEQ_NEW(type); 


/* Definitions for Sequences with commonly used, predefined content-types:*/
DEF_SEQ(int)
DEF_SEQ(float)
DEF_SEQ(double)
DEF_SEQ(char)
DEF_SEQ(bool)