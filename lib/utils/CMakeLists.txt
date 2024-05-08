
add_library(pallasUtils STATIC
    IRVariableCollector.cpp
)
         

target_include_directories(pallasUtils PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(pallasUtils
    ${llvm_libs}  
)