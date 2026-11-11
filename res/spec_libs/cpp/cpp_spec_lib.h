#ifndef PALLAS_CPPSPECLIB_H
#define PALLAS_CPPSPECLIB_H

#include <cstdint>
#include <cstddef>

namespace pallasSpec
{
    /***************************************************
     *                    Datatypes                    *
     ***************************************************/
    // We need to fill the structs with dummy-values so that they are not uniqued with a user's type. 
    
    // Fractions
    struct frac {
        int64_t dummy[0x66726163];
    };

    // ADT for sequences 
    // We use the first element of the struct to encode the content-type
    template<typename T>
    struct seq
    {
        T contentType;
        int64_t dummy[0x736571];
    };


    /***************************************************
     *                    Operators                    *
     ***************************************************/
    
    bool _imply(bool lhs, bool rhs);

    bool _and(bool lhs, bool rhs);

    bool _or(bool lhs, bool rhs);

    bool _sep(bool lhs, bool rhs);

    /***************************************************
     *                Special expressions              *
     ***************************************************/
    template<typename T> 
    T _old(T v);

    template<typename T>
    T _result();

    template<typename T>
    const T& _resultRef();

    /***************************************************
     *                Predicates                       *
     ***************************************************/
    template<typename T>
    T _unfolding(bool pred, T in);

    /***************************************************
     *                    Quantifiers                  *
     ***************************************************/
    template<typename T>
    T _bv(const char* id);

    bool _exists(bool range, bool expr);
    bool _forall(bool range, bool expr);
    bool _forallS(bool range, bool expr);

    /***************************************************
     *                     Fractions                   *
     ***************************************************/
    frac _fracOf(int numerator, int denominator);

    /***************************************************
     *              Permissions & Pointers             *
     ***************************************************/
    template<typename T>
    bool _Perm(T *loc, frac p);

    template<typename T>
    std::size_t _ptrBlockLength(T *loc);

    template<typename T>
    std::size_t _ptrBlockOffset(T *loc);

    template<typename T>
    std::size_t _ptrLength(T *loc);

    /***************************************************
     *              Sequence-operations                *
     ***************************************************/
    template<typename T>
    seq<T> _seqNew();

    template<typename T>
    std::size_t _seqSize(seq<T> s);

    template<typename T>
    T _seqGet(seq<T> s, std::size_t idx);

    template<typename T>
    seq<T> _seqPrepend(T e, seq<T> s);

    template<typename T>
    bool _seqEquals(seq<T> s1, seq<T> s2);

    template<typename T>
    seq<T> _seqSlice(seq<T> s, std::size_t sIdx, std::size_t eIdx);

    template<typename T>
    seq<T> _seqSet(seq<T> s, std::size_t idx, T value);

    
    /***************************************************
     *              Convenience macros                 *
     ***************************************************/
    #define _writePerm pallasSpec::_fracOf(1, 1)

    #define _inRange(lower, v, upper) pallasSpec::_and(lower <= v , v < upper)

    #define _seqHead(s) _seqGet(s, 0)

    #define _seqTail(s) _seqSlice(s, 1, _seqSize(s))
}


#endif // PALLAS_CPPSPECLIB_H