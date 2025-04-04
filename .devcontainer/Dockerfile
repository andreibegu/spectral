FROM mcr.microsoft.com/devcontainers/base:noble
# Setup for LLVM
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

# Setup Swift 6
# See https://www.swift.org/install/linux/tarball/
# Based on https://github.com/swiftlang/swift-docker/blob/main/6.0/ubuntu/24.04/Dockerfile

# Install dependencies 
RUN apt-get -q install -y \
        binutils \
        git \
        gnupg2 \
        libc6-dev \
        libcurl4-openssl-dev \
        libedit2 \
        libgcc-13-dev \
        libncurses-dev \
        libpython3-dev \
        libsqlite3-0 \
        libstdc++-13-dev \
        libxml2-dev \
        libz3-dev \
        pkg-config \
        tzdata \
        unzip \
        zlib1g-dev \ 
        curl
ENV SWIFT_SIGNING_KEY=52BB7E3DE28A71BE22EC05FFEF80A866B47A981F
ENV SWIFT_URL=https://download.swift.org/swift-6.0.1-release/ubuntu2404/swift-6.0.1-RELEASE/swift-6.0.1-RELEASE-ubuntu24.04.tar.gz
ENV SWIFT_SIG_URL=$SWIFT_URL.sig

# Download swift & signature
RUN export GNUPGHOME="$(mktemp -d)"
RUN curl -fsSL "$SWIFT_URL" -o swift6.tar.gz "$SWIFT_SIG_URL" -o swift6.tar.gz.sig
# Get signing keys for swift
RUN gpg --quiet --keyserver hkp://keyserver.ubuntu.com --recv-keys "$SWIFT_SIGNING_KEY"
# Verify signature
RUN gpg --verify swift6.tar.gz.sig swift6.tar.gz
# Unpack & add to path
RUN tar -xzf swift6.tar.gz --directory / --strip-components=1
RUN chmod -R o+r /usr/lib/swift
# Remove archive
RUN rm -rf "$GNUPGHOME" swift6.tar.gz.sig swift6.tar.gz