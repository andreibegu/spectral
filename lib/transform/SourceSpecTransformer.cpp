#include <iostream>

#include "transform/SourceSpecTransformer.h"
#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"

namespace sourcespec2ir
{

    void SourceSpecTransformer::printSpecifications(char *inputPath)
    {
        
    }

    void SourceSpecTransformer::test(char *inputPath)
    {

        std::ifstream srcFileStream;
        srcFileStream.open(inputPath);

        antlr4::ANTLRInputStream inStream(srcFileStream);
        SpecLexer lexer(&inStream);
        antlr4::CommonTokenStream tokens(&lexer);

        /*
        std::cout << "TOKENS: " << std::endl;
        tokens.fill();
        for (auto token : tokens.getTokens()) {
            std::cout << token->toString() << std::endl;
            // std::cout << token->getChannel() << " - " << token->getText() << " - " << token->getType() << std::endl;
        }
        */

        SpecParser parser(&tokens);
        antlr4::tree::ParseTree *tree = parser.file();

        std::cout << tree->toStringTree(&parser) << std::endl
                  << std::endl;

        std::cout.flush();
    }

}