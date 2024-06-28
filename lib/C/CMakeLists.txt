# Library that contains the C-specific aspects of the specification translation
add_library(spec2ir_C STATIC
    CCompilerDriver.cpp
    CSpecTransformer.cpp
    CSpecUnifier.cpp
    CWrapperGenerator.cpp
)
         
target_include_directories(spec2ir_C PUBLIC
    "${CMAKE_SOURCE_DIR}/include/C"
)

target_link_libraries(spec2ir_C
        ${llvm_libs} 
        spec2ir_core
)