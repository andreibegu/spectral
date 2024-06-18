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
> cmake --build .build --target pallasSpec2ir

## Structure
The structure of the repository is based on 
- [VCLLVM](https://github.com/Drevanoorschot/VCLLVM)
- [LLVM Source Tree Layout](https://llvm.org/docs/Projects.html#source-tree-layout)


## Project setup in VSCode

- Ensure that the following packages are installed:
    - cmake
    - zlib1g-dev
    - ninja-build
    - llvm-15-dev
    - clang-15
    - build-essential
    - gdb
- Install the C/C++ Extension Pack for VSCode
- Clone repo
- Open repo-folder in VSCode
- In VSCode:
	- Select gcc as build-kit for CMake
    - Check that Ninja is used as the cmake-generator
	- CMake configure
	- CMake build
	- Add program arguments to the cmake-debug-config (CMake-Options --> Debug Config, or edit directly in the user settings.json-file):
    ```
	"cmake.debugConfig": {
    	"args": ["${workspaceFolder}/examples/spec_seq.c", "-o", "${workspaceFolder}/.tmp/spec2ir_out.ll", "-d"]
    },
    ``` 
	