#include <iostream>

#include "antlr4-runtime.h"
#include "SpecLexer.h"
#include "SpecParser.h"

int main(int argc, char **argv) {
    antlr4::ANTLRInputStream inputString(u8"Here is some random text!!!");
    pallasspec2ir::SpecLexer lexer(&inputString);
    antlr4::CommonTokenStream tokens(&lexer);

    tokens.fill();
    for (auto token : tokens.getTokens()) {
        std::cout << token->toString() << std::endl;
    }

    pallasspec2ir::SpecParser parser(&tokens);
    antlr4::tree::ParseTree* tree = parser.file();

    std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

    // std::cout << " Leberkäse " << std::endl;
    std::cout.flush();
    return 0;

}