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
        size_t line;
        size_t column;

        SrcLocation(size_t line, size_t column);
        SrcLocation(const SrcLocation &baseLoc, size_t colOffset);

        inline size_t getLine() { return line; }
        inline size_t getColumn() { return column; }
    };

    /**
     * A struct that represents a range between a start- and an end-location in
     * a source-file.
     */
    struct SrcRange
    {
        SrcLocation startLoc;
        SrcLocation endLoc;

        SrcRange(size_t startLine, size_t startColumn,
                 size_t endLine, size_t endColumn);

        inline SrcLocation *getStart() { return &startLoc; }
        inline SrcLocation *getEnd() { return &endLoc; }
        inline size_t getStartLine() { return startLoc.getLine(); }
        inline size_t getStartCol() { return startLoc.getColumn(); }
        inline size_t getEndLine() { return endLoc.getLine(); }
        inline size_t getEndCol() { return endLoc.getColumn(); }
    };
}

#endif // PALLAS_SRCLOCATION_H