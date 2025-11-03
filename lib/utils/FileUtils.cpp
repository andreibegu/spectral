#include "FileUtils.h"
#include "PallasException.h"

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

namespace sourcespec2ir::utils
{

    std::string makeAbsPath(std::string path)
    {
        if (llvm::sys::path::is_absolute(path))
            return path;

        llvm::SmallString<512> pathVec = llvm::StringRef(path);
        llvm::sys::fs::make_absolute(pathVec);
        auto absPath = llvm::sys::path::remove_leading_dotslash(pathVec);
        return absPath.str();
    }

    bool isFromFile(const llvm::DISubprogram &sProg,
                    const std::string &file)
    {
        const auto specFilePath = makeAbsPath(file);
        // Build a scope for the source-file where the function is defined.
        // Some compilers put an absolute-path into the filename-field, while
        // others split the path between the filename- and the directory-field.
        auto fPath = sProg.getFilename().str();
        if (!llvm::sys::path::is_absolute(fPath))
        {
            const auto fFile =
                llvm::sys::path::remove_leading_dotslash(sProg.getFilename());
            const auto fDir = sProg.getDirectory();
            const auto fFullPath =
                fDir + llvm::sys::path::get_separator() + fFile;
            fPath = makeAbsPath(
                llvm::sys::path::remove_leading_dotslash(fFullPath.str()).str());
        }

        return fPath == specFilePath;
    }

    
    std::string getChecksumForFile(const std::string &path)
    {
        auto absPath = utils::makeAbsPath(path);
        auto md5Res = llvm::sys::fs::md5_contents(absPath);
        if (!md5Res)
            throw utils::PallasException(
                "Failed to get checksum for file at: " + path);
        return md5Res.get().digest().str().str();
    }
}
