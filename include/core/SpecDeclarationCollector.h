#ifndef PALLAS_SPECDECLARATIONCOLLECTOR_H
#define PALLAS_SPECDECLARATIONCOLLECTOR_H

#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"

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
        SpecDeclarationCollector(SpecParser::FileContext *specParseTree);

        std::unique_ptr<llvm::SmallVector<std::string>> collectDeclarations();

    private:

        SpecParser::FileContext *specParseTree;
        std::unique_ptr<llvm::SmallVector<std::string>> declarations;

        // Overwritten visitor-methods
        std::any visitDeclarationClause(SpecParser::DeclarationClauseContext *ctx) override;
    };

}

#endif // PALLAS_SPECDECLARATIONCOLLECTOR_H