# Pallas_Spec2IR
Transformation from source-level specifications to specifications on IR-level.

## Build
- CMake build based on the [LLVM-CMake guide](https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project) and [VCLLVM](https://github.com/Drevanoorschot/VCLLVM/blob/main/CMakeLists.txt)
- Build using cmake:
> cmake -G "Unix Makefiles" -B .build -DCMAKE_C_COMPILER=/usr/bin/clang-15 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-15    
> cmake --build .build --target pallas-spec2ir

## Structure
The structure of the repository is based on 
- [VCLLVM](https://github.com/Drevanoorschot/VCLLVM)
- [LLVM Source Tree Layout](https://llvm.org/docs/Projects.html#source-tree-layout)