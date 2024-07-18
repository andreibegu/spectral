#include "wrapper/SpecParserWrapper.h"

namespace sourcespec2ir
{

    SpecParserWrapper::SpecParserWrapper(const std::string &srcFilePath)
        : srcFileStream(std::ifstream(srcFilePath)),
          srcInStream(srcFileStream),
          srcLexer(&srcInStream),
          srcTokens(&srcLexer),
          srcParser(&srcTokens)
    {
        srcParseTree = srcParser.file();
    }

    SpecParser::FileContext *SpecParserWrapper::getSrcParseTree()
    {
        return srcParseTree;
    }
}