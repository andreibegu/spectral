#include "wrapper/SrcLocation.h"

namespace sourcespec2ir
{
    SrcLocation::SrcLocation(size_t line, size_t column, std::string file)
        : line(line), column(column), file(file)
    {
    }

    SrcLocation::SrcLocation(const SrcLocation &baseLoc, size_t colOffset,
                             std::string file)
        : line(baseLoc.line), column(baseLoc.column + colOffset), file(file)
    {
    }

    SrcRange::SrcRange(size_t startLine, size_t startColumn,
                       size_t endLine, size_t endColumn, std::string file)
        : startLoc(startLine, startColumn, file),
          endLoc(endLine, endColumn, file)
    {
    }

}