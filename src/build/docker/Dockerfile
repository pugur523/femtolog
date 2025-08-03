# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

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

COPY ${SCRIPTS_DIR} ${SCRIPTS_DIR}
COPY pyproject.toml .
COPY poetry.lock .

RUN curl -sSL https://install.python-poetry.org | python3 - \
    && export PATH="/root/.local/bin:$PATH" \
    && poetry install

COPY ${THIRD_PARTY_DIR} ${THIRD_PARTY_DIR}
COPY . .

RUN export PATH="/root/.local/bin:$PATH" \
    && poetry run python -u ${SCRIPTS_DIR}/build.py \
        --build_mode=all \
        --cpplint \
        --no-clang_format \
        --clang_tidy \
        --no-fail_fast \
        --extra_args="-DFEMTOLOG_ENABLE_BUILD_REPORT=true,-DFEMTOLOG_ENABLE_COVERAGE=true,-DFEMTOLOG_ENABLE_OPTIMIZATION_REPORT=true,-DFEMTOLOG_ENABLE_XRAY=false,-DFEMTOLOG_ENABLE_SANITIZERS=false"
