FROM archlinux:latest

ARG SCRIPTS_DIR=./src/build/scripts
ARG THIRD_PARTY_DIR=./src/third_party

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm \
        base-devel \
        git \
        wget \
        curl \
        gnupg \
        python \
        python-pip \
        ninja \
        nasm \
        yasm \
        pkgconf \
        openssl \
        llvm \
        clang \
        clang-tools-extra \
        lld \
        llvm-libs \
        cmake \
    && pacman -Scc --noconfirm

RUN useradd -m builduser \
    && echo "builduser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER builduser
WORKDIR /home/builduser

RUN git clone https://aur.archlinux.org/yay.git /tmp/yay \
    && (cd /tmp/yay && makepkg -si --noconfirm) \
    && yay -S --noconfirm libc++-with-libunwind

USER root
    
RUN pacman -Scc --noconfirm \
    && rm -rf /tmp/yay

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
