FROM mcr.microsoft.com/devcontainers/base:noble
RUN apt-get update && apt-get -y install \
        openjdk-17-jre \
        build-essential \
        libzstd-dev \
        zlib1g-dev \ 
        gdb \
        cmake \
        ninja-build \ 
        llvm-17-dev \
        clang-17
# TODO: Install Swift 6