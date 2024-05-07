
add_library(sourceSpec2IR
    SourceSpecTransformer.cpp
)

target_include_directories(sourceSpec2IR PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    ${ANTLR_SpecificationParser_OUTPUT_DIR} # Is this required here?
)




#include_directories(${ANTLR_SpecificationLexer_OUTPUT_DIR})
#include_directories(${ANTLR_SpecificationParser_OUTPUT_DIR})


#target_link_libraries(sourceSpec2IR
#        antlr4_static 
#)