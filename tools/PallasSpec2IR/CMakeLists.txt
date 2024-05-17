set(SOURCE_FILES PallasSpec2IR.cpp)


add_executable(pallasSpec2ir 
        ${SOURCE_FILES} 
)
target_include_directories(pallasSpec2ir
        PRIVATE "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(pallasSpec2ir 
        ${llvm_libs} 
        sourceSpec2IR
)