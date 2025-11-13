#ifndef PALLAS_SRCTYPEREPRESENTATION_H
#define PALLAS_SRCTYPEREPRESENTATION_H

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>

namespace sourcespec2ir
{

    /**
     * Struct that represents a string-representation of a type on the level of
     * the source language.
     * The rperesentation consists of a prefic and a suffix.
     * In most cases, only the prefix is used.
     * The suffix is mostly required for e.g. arrays in C (e.g. int a[4])
     * where 'int' is the prefix and '[4]' is the suffix.
     */
    struct SrcTypeRepresentation
    {
        llvm::SmallString<16> prefix;
        llvm::SmallString<16> suffix;

        SrcTypeRepresentation(llvm::StringRef prefix, llvm::StringRef suffix = "");
    };
}

#endif // PALLAS_SRCTYPEREPRESENTATION_H