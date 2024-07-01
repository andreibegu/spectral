set(SOURCE_FILES PallasSpec2IR.cpp)


add_executable(pallasSpec2ir 
        ${SOURCE_FILES} 
)
target_include_directories(pallasSpec2ir
        PRIVATE "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(pallasSpec2ir 
        spec2ir_core
        spec2ir_C
        spec2ir_Cpp
)