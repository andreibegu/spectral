
# Generation of lexer & arser for the specification-grammar
antlr_target(SpecificationLexer ${CMAKE_SOURCE_DIR}/grammar/SpecLexer.g4 LEXER
             PACKAGE sourcespec2ir
             COMPILE_FLAGS -Xexact-output-dir)
antlr_target(SpecificationParser ${CMAKE_SOURCE_DIR}/grammar/SpecParser.g4 PARSER
             PACKAGE sourcespec2ir
             DEPENDS_ANTLR SpecificationLexer
             COMPILE_FLAGS -lib ${ANTLR_SpecificationLexer_OUTPUT_DIR} -Xexact-output-dir
             VISITOR)


add_library(sourceSpec2IR STATIC
    SourceSpecTransformer.cpp
    DummySourceSpecContextProvider.cpp
    CCompilerDriver.cpp
    ${ANTLR_SpecificationLexer_CXX_OUTPUTS} 
    ${ANTLR_SpecificationParser_CXX_OUTPUTS} 
)
         

target_include_directories(sourceSpec2IR PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
    "${ANTLR_SpecificationParser_OUTPUT_DIR}"
    "${ANTLR_SpecificationLexer_OUTPUT_DIR}"
)

target_link_libraries(sourceSpec2IR
        antlr4_static 
        pallasUtils
)