#ifndef PALLAS_FILEUTILS_H
#define PALLAS_FILEUTILS_H

#include <string>
#include <llvm/IR/DebugInfoMetadata.h>

namespace sourcespec2ir
{
    namespace utils
    {
        /**
         * Uses llvm::sys::fs::make_absolute to turn the given path 
         * into an absolute path.
         */
        std::string makeAbsPath(std::string path);

        /**
         * Checks if the given DISubprogram is defined in the given file.
         * Takes care of relative / absolute paths.
         */
        bool isFromFile(const llvm::DISubprogram &sProg,
                        const std::string &file);
    }

}

#endif // PALLAS_FILEUTILS_H