# Pallas_Spec2IR
Transformation from source-level specifications to specifications on IR-level.


**TODO: Add instructions on how to use the tool standalone and on how to build the swift specification-library.**

## Requirements:
- Installation of antlr4 Cpp-target
https://github.com/antlr/antlr4/tree/master/runtime/Cpp

## Build
- CMake build based on the [LLVM-CMake guide](https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project) and [VCLLVM](https://github.com/Drevanoorschot/VCLLVM/blob/main/CMakeLists.txt)
- Antlr4-build is based on the [CMake-template for the cpp-target](https://github.com/antlr/antlr4/tree/master/runtime/Cpp/cmake)
- Build using cmake:
> cmake -G "Ninja" -B .build -DCMAKE_C_COMPILER=/usr/bin/clang-17 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-17    
> cmake --build .build --target pallasSpec2ir

## Structure
The structure of the repository is based on 
- [VCLLVM](https://github.com/Drevanoorschot/VCLLVM)
- [LLVM Source Tree Layout](https://llvm.org/docs/Projects.html#source-tree-layout)

## Project-setup using DevContainer

### Setup the DevContainer
- Install Docker, Visual Studio Code
- Install the Devcontainer-extension in VSCode
- Clone the repository 
- Open folder of the cloned repository in VSCode
- Press 'Ctrl-Shift-P' and select 'Reopen in Container'
    - This may take a few minutes as it builds the container

### Setup the CMake extension
- Press ```Ctrl-Shift-P``` and select ```CMake: Scan for Kits```
- Press ```Ctrl-Shift-P``` and select ```CMake: Select a Kit```
    - Select the listed GCC-kit
    - Optionally, you can also select clang as a kit, but this works less good during debugging
- Press ```Ctrl-Shift-P``` and select ```CMake: Configure```

By default, we use ninja as the CMake generator. 

### Build using CMake:
- Press ```Ctrl-Shift-P``` and select ```CMake: Build```
This will build the specification-translation framework. 

### Setup debug-configuration: 
To setup and modify the run- and debug-configuration for the CMake extension, 
add or modify the following entry to your settings.json:
(Type ```Ctrl-Shift-P``` and select 'Preferences: Open Workspace Settings (JSON)')

```
"cmake.debugConfig": {
    "args": [
        "${workspaceFolder}/examples/spec_demo.c", 
        "-o", "${workspaceFolder}/.tmp/spec_demo.ll", 
        "-d",
        "-m",
        "-lib", "${workspaceFolder}/res/spec_libs/c"
    ]
}
``` 

Here you can specify the arguments that are passed to the application when debugging it from VSCode. 

To run the debug-configuration, type ```Ctrl-Shift-P``` and select ```CMake: Debug```.


## Project setup in VSCode
- Ensure that the following packages are installed:
    - build-essential
    - gdb
    - cmake
    - zlib1g-dev
    - libzstd-dev
    - ninja-build
    - llvm-17-dev
    - clang-17
- If you want to use the swift-frontend, make sure that Swift 6.0 is installed and available on your PATH.
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
        "args": [
            "${workspaceFolder}/examples/spec_while.c", 
            "-o", "${workspaceFolder}/.tmp/spec2ir_out.ll", 
            "-d",
            "-m",
            "-lib", "${workspaceFolder}/res/spec_libs/c"
        ]
    }
    ``` 

## Testing:
For running regression tests we use the [lit tool](https://llvm.org/docs/CommandGuide/lit.html) of the LLVM project. This requires a python version >= 3.8 to be installed. 
Lit can be installed as a [python-package](https://pypi.org/project/lit/) using `pip install lit==18.1.8` or as part of an LLVM build. 
In newer versions, installing the lit-package into the default python environment 
does not work. In this case, a python-venv with lit can be created 
by running `ninja makeLitVenv` in the build-directory.

To run the tests, run `ninja check` in the build-directory.
If a virtual environment is used for lit (using `ninja makeLitVenv`), run `ninja checkV` instead.
If the terminal in VSCode is used to run the tests in WSL and lit cannot be found 
right after installing it, it can be nessessary to reboot the system once. 

The lit-setup is based on:
- https://github.com/llvm/llvm-project/tree/main/mlir/examples/standalone/test
- [VCLLVM](https://github.com/Drevanoorschot/VCLLVM/tree/main/test)


## Building the Swift specification-library
To build the specification-library for Swift, run `ninja BuildSwiftLib` in the build-directory of the project. 
The library is build in `res/swift/PallasSpec/.build/debug/`.

	