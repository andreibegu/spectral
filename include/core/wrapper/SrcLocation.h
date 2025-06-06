#ifndef PALLAS_SRCLOCATION_H
#define PALLAS_SRCLOCATION_H

#include <cstddef>

namespace sourcespec2ir
{

    /**
     * A struct that represents a location in a source-file.
     */
    struct SrcLocation
    {
        const size_t line;
        const size_t column;

        SrcLocation(size_t line, size_t column);
        SrcLocation(const SrcLocation &baseLoc, size_t colOffset);

        inline size_t getLine() const { return line; }
        inline size_t getColumn() const { return column; }
    };

    /**
     * A struct that represents a range between a start- and an end-location in
     * a source-file.
     */
    struct SrcRange
    {
        const SrcLocation startLoc;
        const SrcLocation endLoc;

        SrcRange(size_t startLine, size_t startColumn,
                 size_t endLine, size_t endColumn);

        inline size_t getStartLine() const { return startLoc.getLine(); }
        inline size_t getStartCol() const { return startLoc.getColumn(); }
        inline size_t getEndLine() const { return endLoc.getLine(); }
        inline size_t getEndCol() const { return endLoc.getColumn(); }
    };
}

#endif // PALLAS_SRCLOCATION_H