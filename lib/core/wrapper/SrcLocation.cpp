#include "wrapper/SrcLocation.h"

namespace sourcespec2ir
{
  SrcLocation::SrcLocation(const size_t line, const size_t column,
                           const std::string &file,
                           const std::string &md5Checksum)
      : line(line), column(column), file(file), md5Checksum(md5Checksum)
  {
  }

  SrcLocation::SrcLocation(const SrcLocation &baseLoc, const size_t colOffset,
                           const std::string &file,
                           const std::string &md5Checksum)
      : line(baseLoc.line), column(baseLoc.column + colOffset), file(file),
        md5Checksum(md5Checksum)
  {
  }

  std::string SrcLocation::asShortString() const
  {
    return std::to_string(line) + "::" + std::to_string(column);
  }

  SrcRange::SrcRange(const size_t startLine, const size_t startColumn,
                     const size_t endLine, const size_t endColumn,
                     const std::string &file, const std::string &md5Checksum)
      : startLoc(startLine, startColumn, file, md5Checksum),
        endLoc(endLine, endColumn, file, md5Checksum)
  {
  }

  std::string SrcRange::asShortString(bool lineOnly) const
  {
    if (lineOnly)
      return std::to_string(getStartLine()) + "-" +
             std::to_string(getEndLine());

    return startLoc.asShortString() + "-" + endLoc.asShortString();
  }

}