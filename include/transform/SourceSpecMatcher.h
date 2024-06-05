#ifndef PALLAS_SOURCESPECMATCHER_H
#define PALLAS_SOURCESPECMATCHER_H

#include "MatchedSourceSpec.h"
#include "SpecParser.h"
#include "SpecParserBaseVisitor.h"

#include <memory>

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Module.h>

namespace sourcespec2ir
{

    /**
     * Class that matches the contracts from the parse-tree of the source-file to the
     * corresponding constructs (functions, loops, instructions) in the IR module.
     *
     * Does not take ownership of the parse-tree or the IR-module.
     */
    class SourceSpecMatcher : SpecParserBaseVisitor
    {
    public:
        SourceSpecMatcher(SpecParser::FileContext *specParseTree, llvm::Module *irModule);

        std::unique_ptr<llvm::SmallVector<std::unique_ptr<MatchedFunctionContract>>> matchSpecifications();

    protected:

        SpecParser::FileContext *specParseTree;
        llvm::Module *irModule;

        /**
         * Given the location of a specContractBlock in the source-file, this mehtod
         * returns the name of the funcion that the specification refers to. 
         * TODO: Check if it makes more sense to return a pointer to the function instead of the name.
         */
        virtual std::string getContractBlockContext(size_t startLine, size_t startCol, size_t endLine, size_t endCol) = 0;

    private:

        // Temporary pointer to collection in which the matches specifications are collected before being returned.
        std::unique_ptr<llvm::SmallVector<std::unique_ptr<MatchedFunctionContract>>> matchedSpecifications;

        // Overwritten visitor-methods
        std::any visitSpecContractBlock(SpecParser::SpecContractBlockContext *ctx) override;
    };

}

#endif // PALLAS_SOURCESPECMATCHER_H