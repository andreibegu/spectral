#include <iostream>

#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"
#include "transform/SourceSpecTransformer.h"

int main(int argc, char **argv) {
    std::ifstream srcFileStream;
    srcFileStream.open(argv[1]);

    // antlr4::ANTLRInputStream inputString(u8"Here is some random text!!!");
    antlr4::ANTLRInputStream inStream(srcFileStream);
    pallasspec2ir::SpecLexer lexer(&inStream);
    antlr4::CommonTokenStream tokens(&lexer);

    /*
    std::cout << "TOKENS: " << std::endl;
    tokens.fill();
    for (auto token : tokens.getTokens()) {
        std::cout << token->toString() << std::endl;
        // std::cout << token->getChannel() << " - " << token->getText() << " - " << token->getType() << std::endl;
    }
    */

    // std::cout << "TREE: " << std::endl;
    pallasspec2ir::SpecParser parser(&tokens);
    antlr4::tree::ParseTree* tree = parser.file();
    

    std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

    auto specTransformer = sourcespec2ir::SourceSpecTransformer();
    specTransformer.test();

    // std::cout << " Leberkäse " << std::endl;
    std::cout.flush();
    return 0;

}