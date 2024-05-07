set(SOURCE_FILES PallasSpec2IR.cpp)


# Specification-grammar
antlr_target(SpecificationLexer ${CMAKE_SOURCE_DIR}/grammar/SpecLexer.g4 LEXER
             PACKAGE pallasspec2ir
             COMPILE_FLAGS -Xexact-output-dir)
antlr_target(SpecificationParser ${CMAKE_SOURCE_DIR}/grammar/SpecParser.g4 PARSER
             PACKAGE pallasspec2ir
             DEPENDS_ANTLR SpecificationLexer
             COMPILE_FLAGS -lib ${ANTLR_SpecificationLexer_OUTPUT_DIR} -Xexact-output-dir
             VISITOR)

include_directories(${ANTLR_SpecificationLexer_OUTPUT_DIR})
include_directories(${ANTLR_SpecificationParser_OUTPUT_DIR})


add_executable(pallas-spec2ir 
        ${SOURCE_FILES} 
        ${ANTLR_SpecificationLexer_CXX_OUTPUTS} 
        ${ANTLR_SpecificationParser_CXX_OUTPUTS} 
)
target_include_directories(pallas-spec2ir
        PRIVATE "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(pallas-spec2ir 
        ${llvm_libs} 
        antlr4_static 
        sourceSpec2IR
)