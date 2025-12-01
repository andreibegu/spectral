#ifndef PALLAS_SPECPREDICATECOLLECTOR_H
#define PALLAS_SPECPREDICATECOLLECTOR_H

#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"
#include "SpecPredicate.h"

#include <memory>
#include <string>

#include <llvm/ADT/SmallVector.h>

namespace sourcespec2ir
{

    /**
     * Collects all predicate definitions from the source file.
     */
    class SpecPredicateCollector : SpecParserBaseVisitor
    {
    public:
        SpecPredicateCollector(SpecParser::FileContext &specParseTree,
                               const std::string &file,
                               const std::string &md5Checksum,
                               const bool printInfo);

        std::unique_ptr<llvm::SmallVector<SpecPredicate, 16>> collect();

    private:
        SpecParser::FileContext &specParseTree;
        const std::string file;
        const std::string md5Checksum;
        const bool printInfo;
        std::unique_ptr<llvm::SmallVector<SpecPredicate, 16>> predicates;

        // Overwritten visitor-methods
        std::any visitPredicateDef(
            SpecParser::PredicateDefContext *ctx) override;
    };

}

#endif // PALLAS_SPECPREDICATECOLLECTOR_H