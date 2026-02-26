# Library that contains the C++-specific aspects of the specification translation
add_library(spec2ir_Swift STATIC
    SwiftCompilerDriver.cpp
    SwiftDemangler.cpp
    SwiftWrapperGenerator.cpp
    SwiftSpecTransformer.cpp
    SwiftSpecUnifier.cpp
    SwiftSyntaxResolver.cpp
)
         
target_include_directories(spec2ir_Swift PUBLIC
    "${CMAKE_SOURCE_DIR}/include/swift"
)

target_link_libraries(spec2ir_Swift
        ${llvm_libs} 
        spec2ir_core
)