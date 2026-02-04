#ifndef PALLAS_SRCLOCATION_H
#define PALLAS_SRCLOCATION_H

#include <cstddef>
#include <string>

namespace sourcespec2ir
{

    /**
     * A struct that represents a location in a source-file.
     */
    struct SrcLocation
    {
        const size_t line;
        const size_t column;
        const std::string file;
        const std::string md5Checksum;

        SrcLocation(const size_t line, const size_t column, 
            const std::string &file, const std::string &md5Checksum);
        SrcLocation(const SrcLocation &baseLoc, size_t colOffset,
                    const std::string &file, const std::string &md5Checksum);

        inline size_t getLine() const { return line; }
        inline size_t getColumn() const { return column; }
        inline const std::string &getFile() const { return file; }
        inline const std::string &getMD5() const { return md5Checksum; }
        std::string asShortString() const;
    };

    /**
     * A struct that represents a range between a start- and an end-location in
     * a source-file.
     */
    struct SrcRange
    {
        const SrcLocation startLoc;
        const SrcLocation endLoc;

        SrcRange(const size_t startLine, const size_t startColumn,
                 const size_t endLine, const size_t endColumn, 
                 const std::string &file, const std::string &md5Checksum);

        inline size_t getStartLine() const { return startLoc.getLine(); }
        inline size_t getStartCol() const { return startLoc.getColumn(); }
        inline size_t getEndLine() const { return endLoc.getLine(); }
        inline size_t getEndCol() const { return endLoc.getColumn(); }
        inline const std::string &getFile() const { return startLoc.getFile(); }
        inline const std::string &getMD5() const { return startLoc.getMD5(); }
        std::string asShortString(bool lineOnly=true) const;
    };
}

#endif // PALLAS_SRCLOCATION_H