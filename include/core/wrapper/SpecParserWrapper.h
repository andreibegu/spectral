#ifndef PALLAS_SPECPARSERWRAPPER_H
#define PALLAS_SPECPARSERWRAPPER_H

#include <memory>
#include <string>

#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"

namespace sourcespec2ir
{

    /**
     * Convinience-class that manages the streams, lexer, parser that are 
     * required for parsing the specifications from the source-file. 
     * The class takes ownership of all the required streams... that are
     * required by the parser. 
     */
    class SpecParserWrapper
    {
        public:
        /**
         * Creates a new parser-wrapper that parses the given source-file.
         */
        SpecParserWrapper(const std::string &srcFilePath);

        /**
         * Returns a pointer to the parse-tree. 
         * Owership stays with the parser-wrapper. The pointer is valid as long 
         * as the parser-wrappers stays alive.  
         */
        SpecParser::FileContext * getSrcParseTree();

        private:
        // TODO: Check if this stream needs to stay alive
        std::ifstream srcFileStream; 
        antlr4::ANTLRInputStream srcInStream;
        SpecLexer srcLexer;
        antlr4::CommonTokenStream srcTokens;
        SpecParser srcParser;
        SpecParser::FileContext *srcParseTree;
    };
}

#endif // PALLAS_SPECPARSERWRAPPER_H