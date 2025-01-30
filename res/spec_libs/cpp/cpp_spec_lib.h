#ifndef PALLAS_CPPSPECLIB_H
#define PALLAS_CPPSPECLIB_H

#include <cstdint>
#include <cstddef>

namespace pallasSpec
{
    /***************************************************
     *                    Datatypes                    *
     ***************************************************/
    // We need to fill the structs with dummy-values so that it exceeds a size 
    // of 64 bytes. Otherwise, the struct will be decomposed into its individual
    // values or compressed into an int when it is passed to a function. 
    
    // Fractions
    struct frac {
        int64_t dummy1, dummy2, dummy3;
    };

    // ADT for sequences 
    // We use the first element of the struct to encode the content-type
    template<typename T>
    struct seq
    {
        T contentType;
        int64_t dummy1, dummy2, dummy3;
    };


    /***************************************************
     *                    Operators                    *
     ***************************************************/
    
    bool imply(bool lhs, bool rhs);

    bool _and(bool lhs, bool rhs);

    bool _or(bool lhs, bool rhs);

    bool sep(bool lhs, bool rhs);

    /***************************************************
     *                Special expressions              *
     ***************************************************/
    template<typename T> 
    T old(T v);

    template<typename T>
    T result();

    /***************************************************
     *                    Quantifiers                  *
     ***************************************************/
    template<typename T>
    T bVar(const char* id);

    bool exists(bool range, bool expr);
    bool forall(bool range, bool expr);
    bool forallSep(bool range, bool expr);

    /***************************************************
     *                     Fractions                   *
     ***************************************************/
    frac fracOf(int numerator, int denominator);

    /***************************************************
     *                   Permissions                   *
     ***************************************************/
    template<typename T>
    bool Perm(T *loc, frac p);

    /***************************************************
     *              Sequence-operations                *
     ***************************************************/
    template<typename T>
    seq<T> seqNew();

    template<typename T>
    std::size_t seqSize(seq<T> s);

    template<typename T>
    T seqGet(seq<T> s, std::size_t idx);

    template<typename T>
    seq<T> seqPrepend(T e, seq<T> s);

    template<typename T>
    bool seqEquals(seq<T> s1, seq<T> s2);
}


#endif // PALLAS_CPPSPECLIB_H