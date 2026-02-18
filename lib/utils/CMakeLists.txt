
add_library(pallasUtils STATIC
    FileUtils.cpp
    SrcLookup.cpp
    IRUtils.cpp
    MetadataUtils.cpp
)
         

target_include_directories(pallasUtils PUBLIC
    "${CMAKE_SOURCE_DIR}/include/utils"
)

target_link_libraries(pallasUtils
    ${llvm_libs}  
)