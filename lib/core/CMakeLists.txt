# Library that contains the source-language independent aspects of the transformation. 

# Generation of lexer & parser for the specification-grammar
antlr_target(SpecificationLexer ${CMAKE_SOURCE_DIR}/grammar/SpecLexer.g4 LEXER
             PACKAGE sourcespec2ir
             COMPILE_FLAGS -Xexact-output-dir)
antlr_target(SpecificationParser ${CMAKE_SOURCE_DIR}/grammar/SpecParser.g4 PARSER
             PACKAGE sourcespec2ir
             DEPENDS_ANTLR SpecificationLexer
             COMPILE_FLAGS -lib ${ANTLR_SpecificationLexer_OUTPUT_DIR} -Xexact-output-dir
             VISITOR)

add_library(spec2ir_core STATIC
    IRBasedSourceSpecMatcher.cpp
    MatchedSourceSpec.cpp
    ParsingUtils.cpp
    SourceCompilerDriver.cpp
    SourceSpecMatcher.cpp
    SourceSpecTransformer.cpp
    SourceSyntaxResolver.cpp
    SourceWrapperGenerator.cpp
    SpecDeclarationCollector.cpp
    SpecEncoder.cpp
    SpecPredicateCollector.cpp
    SpecUnifier.cpp
    WrapperLocCollector.cpp
    ${ANTLR_SpecificationLexer_CXX_OUTPUTS} 
    ${ANTLR_SpecificationParser_CXX_OUTPUTS} 
)

add_subdirectory(wrapper)
add_subdirectory(passes)
         
target_include_directories(spec2ir_core PUBLIC
    # TODO: Be more specific
    "${CMAKE_SOURCE_DIR}/include/core"
    "${ANTLR_SpecificationParser_OUTPUT_DIR}"
    "${ANTLR_SpecificationLexer_OUTPUT_DIR}"
)

target_link_libraries(spec2ir_core
        ${llvm_libs} 
        antlr4_static 
        pallasUtils
)