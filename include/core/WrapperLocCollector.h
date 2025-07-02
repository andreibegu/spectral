#ifndef PALLAS_WRAPPERLOCCOLLECTOR_H
#define PALLAS_WRAPPERLOCCOLLECTOR_H

#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"
#include "wrapper/WrapperLocMarker.h"

#include <memory>
#include <string>

#include <llvm/ADT/SmallVector.h>

namespace sourcespec2ir
{

    /**
     * Class that collects all marker-annotations for insert-locations
     * of wrapper-functions.
     */
    class WrapperLocCollector : SpecParserBaseVisitor
    {
    public:
        WrapperLocCollector(SpecParser::FileContext &specParseTree,
                            const std::string &file, 
                            const std::string &md5Checksum);

        std::unique_ptr<llvm::SmallVector<WrapperLocMarker>>
        collectLocationMarkers();

    private:
        SpecParser::FileContext &specParseTree;
        const std::string file;
        const std::string md5Checksum;
        std::unique_ptr<llvm::SmallVector<WrapperLocMarker>> wrapperLocations;

        // Overwritten visitor-methods
        std::any visitWrapperLocationMarker(
            SpecParser::WrapperLocationMarkerContext *ctx) override;
    };

}

#endif // PALLAS_WRAPPERLOCMARKERCOLLECTOR_H