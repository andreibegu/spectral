#ifndef PALLAS_SPECDECLARATIONCOLLECTOR_H
#define PALLAS_SPECDECLARATIONCOLLECTOR_H

#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"
#include "SpecDeclaration.h"

#include <memory>

#include <llvm/ADT/SmallVector.h>

namespace sourcespec2ir
{

    /**
     * Class that collects all declarations from the optional declaration-block.
     */
    class SpecDeclarationCollector : SpecParserBaseVisitor
    {
    public:
        SpecDeclarationCollector(SpecParser::FileContext *specParseTree,
                                 const std::string &file,
                                 const std::string &md5Checksum);

        std::unique_ptr<llvm::SmallVector<SpecDeclaration>> collectDeclarations();

    private:
        SpecParser::FileContext *specParseTree;
        const std::string file;
        const std::string md5Checksum;
        std::unique_ptr<llvm::SmallVector<SpecDeclaration>> declarations;

        // Overwritten visitor-methods
        std::any visitDeclarationClause(SpecParser::DeclarationClauseContext *ctx) override;
    };

}

#endif // PALLAS_SPECDECLARATIONCOLLECTOR_H