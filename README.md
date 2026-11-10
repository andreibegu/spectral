# Pallas_Spec2IR
Transformation from source-level specifications to specifications on IR-level.
The documentation can be found [here](Documentation.md).

## Invocation
After building the project using the instructions below, the executable is located under ```/.build/bin/pallasSpec2ir```.
The tool can then be invoked as follows:
> ./.build/bin/pallasSpec2ir SRC_PATH -o OUT_PATH --wDir W_DIR --lib LIB_PATH -m 

where 
- ```SRC_PATH``` is the path to the source-file that should be transformed
- ```OUT_PATH``` is the path where the resulting LLVM-module should be saved
- ```W_DIR``` is an optional path to a working directory where temporary files will be created
- ```LIB_PATH``` is the path to the specification library
- ```-m``` (optional) causes information about the matching of specifications to language-constructs to be printed.

For a full overview of the available options and parameters, run the tool with the ```--help```-option.

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
- If one of the above steps fails, delete the ```.build/``` folder and try again.

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

## Manual setup:
This section outlines how to setup the project manually, without using a DevContainer. 
The build has only been tested on Ubuntu. 
We use cmake and ninja for the build. Our build is based on the 
[LLVM-CMake guide](https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project)
and the 
[CMake-template for the Antlr4 C++-target](https://github.com/antlr/antlr4/tree/master/runtime/Cpp/cmake).


### Clone repository 
Clone the repository. 
If you are working on a Windows-system, we recommend working within WSL2, as the build has only been tested for Ubuntu-based systems. 

### Install dependencies: 
Install the following dependencies (assuming a Ubuntu-based system):
- openjdk-17-jre
- build-essential
- gdb
- cmake
- zlib1g-dev
- libzstd-dev
- ninja-build
- llvm-17-dev
- clang-17

If you want to use the Swift-frontend, make sure that Swift 6.0 is installed and available on your PATH. Additionally, the `swift-demangle` tool must be available on PATH (this is usually included in the default-installation of Swift).

### Build using CMake and Ninja: 
Run the following command to generate the ninja build-files in the ```/.build```-folder:
> cmake -G "Ninja" -B .build -DCMAKE_C_COMPILER=/usr/bin/clang-17 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-17

To build the project, run the following command: 
> cmake --build .build --target pallasSpec2ir

Afterwards, the executable is then located under ```/.build/bin/pallasSpec2ir```.

### (Optional) Setup Visual Studio Code:
- Install the following VSCode extensions: 
    - ms-vscode.cpptools-extension-pack
    - ms-vscode.cmake-tools
- Open the cloned repository in VSCode 
- Follow the steps from the section 'Project-setup using DevContainer' starting at subsection 'Setup the CMake extension'.

## Testing:
For running regression tests we use the [lit tool](https://llvm.org/docs/CommandGuide/lit.html) of the LLVM project. This requires a python version >= 3.8 to be installed. 
Lit can be installed as a python-package using `pip install lit==18.1.8` or as part of an LLVM build. 
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

## Licence 
Copyright (c) 2024 - 2026 Formal Methods and Tools, University of Twente. All rights reserved.

This project is licenced under the Apache License 2.0 as described in LICENSE.txt in the root of this project.
This project uses third-party dependencies. Check THIRD-PARTY-LICENSES.md for the licenses of the included dependencies.