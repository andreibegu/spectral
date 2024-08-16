#include "wrapper/SrcLocation.h"

namespace sourcespec2ir
{
    SrcLocation::SrcLocation(size_t line, size_t column)
        : line(line), column(column)
    {
    }

    SrcLocation::SrcLocation(const SrcLocation &baseLoc, size_t colOffset)
        : line(baseLoc.line), column(baseLoc.column + colOffset)
    {
    }

    SrcRange::SrcRange(size_t startLine, size_t startColumn,
             size_t endLine, size_t endColumn)
        : startLoc(startLine, startColumn), endLoc(endLine, endColumn)
    {
    }

}