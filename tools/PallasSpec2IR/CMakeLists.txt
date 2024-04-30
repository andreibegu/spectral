set(SOURCE_FILES PallasSpec2IR.cpp)
add_executable(pallas-spec2ir ${SOURCE_FILES})
target_include_directories(
        pallas-spec2ir
        PRIVATE
        "${PROJECT_SOURCE_DIR}/include")

target_link_libraries(pallas-spec2ir ${llvm_libs})
# target_link_libraries(pallas-spec2ir ${LLVM_LIBS} VCLLVM_PASSES proto-objects OriginProvider)