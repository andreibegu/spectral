# Library that contains the C++-specific aspects of the specification translation
add_library(spec2ir_Cpp STATIC
    CppCompilerDriver.cpp
    CppSpecTransformer.cpp
    CppSpecUnifier.cpp
    CppWrapperGenerator.cpp
)
         
target_include_directories(spec2ir_Cpp PUBLIC
    "${CMAKE_SOURCE_DIR}/include/Cpp"
)

target_link_libraries(spec2ir_Cpp
        ${llvm_libs} 
        spec2ir_core
)