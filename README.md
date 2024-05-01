# Pallas_Spec2IR
Transformation from source-level specifications to specifications on IR-level.

## Requirements:
- Installation of antlr4 Cpp-target
https://github.com/antlr/antlr4/tree/master/runtime/Cpp

## Build
- CMake build based on the [LLVM-CMake guide](https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project) and [VCLLVM](https://github.com/Drevanoorschot/VCLLVM/blob/main/CMakeLists.txt)
- Antlr4-build is based on the [CMake-template for the cpp-target](https://github.com/antlr/antlr4/tree/master/runtime/Cpp/cmake)
- Build using cmake:
> cmake -G "Unix Makefiles" -B .build -DCMAKE_C_COMPILER=/usr/bin/clang-15 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-15    
> cmake --build .build --target pallas-spec2ir

## Structure
The structure of the repository is based on 
- [VCLLVM](https://github.com/Drevanoorschot/VCLLVM)
- [LLVM Source Tree Layout](https://llvm.org/docs/Projects.html#source-tree-layout)
