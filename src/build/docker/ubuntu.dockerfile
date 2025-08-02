FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

ARG LLVM_VERSION=20
ARG CMAKE_VERSION=4.0.2
ARG CMAKE_OS=linux
ARG CMAKE_ARCH=x86_64
ARG SCRIPTS_DIR=./src/build/scripts
ARG THIRD_PARTY_DIR=./src/third_party

RUN apt-get update && apt-get install -y \
    wget \
    curl \
    gnupg \
    lsb-release \
    software-properties-common \
    ca-certificates \
    git \
    libssl-dev \
    make \
    ninja-build \
    python3 \
    python3-pip \
    nasm \
    yasm \
    pkg-config \
    && wget -O llvm.sh https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh ${LLVM_VERSION} all \
    && rm llvm.sh \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install CMake
RUN CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-${CMAKE_OS}-${CMAKE_ARCH}.sh" \
    && wget -nv "${CMAKE_URL}" -O cmake-installer.sh \
    && chmod +x cmake-installer.sh \
    && ./cmake-installer.sh --skip-license --prefix=/usr/local \
    && rm cmake-installer.sh

RUN ln -s /usr/bin/clang-${LLVM_VERSION} /usr/bin/clang \
    && ln -s /usr/bin/clang++-${LLVM_VERSION} /usr/bin/clang++ \
    && ln -s /usr/bin/clang-tidy-${LLVM_VERSION} /usr/bin/clang-tidy \
    && ln -s /usr/bin/clang-format-${LLVM_VERSION} /usr/bin/clang-format \
    && ln -s /usr/bin/llvm-config-${LLVM_VERSION} /usr/bin/llvm-config \
    && ln -s /usr/bin/llvm-cov-${LLVM_VERSION} /usr/bin/llvm-cov \
    && ln -s /usr/bin/llvm-profdata-${LLVM_VERSION} /usr/bin/llvm-profdata \
    && ln -s /usr/bin/llvm-xray-${LLVM_VERSION} /usr/bin/llvm-xray \
    && ln -s /usr/bin/python3 /usr/bin/python

WORKDIR /app

COPY ${SCRIPTS_DIR}/requirements.txt ${SCRIPTS_DIR}/

RUN pip3 install --break-system-packages --no-cache-dir --ignore-installed -r ${SCRIPTS_DIR}/requirements.txt

COPY ${SCRIPTS_DIR} ${SCRIPTS_DIR}
COPY ${THIRD_PARTY_DIR} ${THIRD_PARTY_DIR}
COPY . /app

RUN python3 -u ${SCRIPTS_DIR}/build.py \
        --build_mode=all \
        --cpplint \
        --no-clang_format \
        --clang_tidy \
        --build_async \
        --no-fail_fast \
        --extra_args="-DFEMTOLOG_ENABLE_BUILD_REPORT=true,-DFEMTOLOG_ENABLE_COVERAGE=true,-DFEMTOLOG_ENABLE_OPTIMIZATION_REPORT=true,-DFEMTOLOG_ENABLE_XRAY=false,-DFEMTOLOG_ENABLE_SANITIZERS=false"
