#!/usr/bin/env python3

# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

import os
import sys
import platform
import subprocess
import shutil


def get_platform_name(operating_system: str = "") -> str:
    if not operating_system:
        operating_system = sys.platform
    if operating_system.startswith("win"):
        return "windows"
    elif operating_system.startswith("linux"):
        return "linux"
    elif operating_system.startswith("darwin"):
        return "darwin"
    else:
        raise RuntimeError(f"Unsupported platform: {operating_system}")


def get_arch_name() -> str:
    return platform.machine().lower()


def get_platform_arch_string() -> str:
    return f"{get_platform_name()}-{get_arch_name()}"


def run_command(argv, *args, **kwargs):
    return subprocess.run(argv, *args, **kwargs)


def is_installed(tool_name: str) -> bool:
    return shutil.which(tool_name) is not None


scripts_dir: str = os.path.dirname(__file__)
project_root_dir: str = os.path.abspath(os.path.join(scripts_dir, "..", "..", ".."))
project_src_dir: str = os.path.join(project_root_dir, "src")
project_config_file: str = os.path.join(project_root_dir, "project_config.toml")
third_party_src_dir: str = os.path.join(project_src_dir, "third_party")

out_root_dir: str = os.path.join(project_root_dir, "out")
build_root_dir: str = os.path.join(out_root_dir, "build")
install_root_dir: str = os.path.join(out_root_dir, "install")


def build_platform_dir(platform="", arch=""):
    return os.path.join(
        build_root_dir,
        platform or get_platform_name(),
        arch or get_arch_name(),
    )


def install_platform_dir(platform="", arch=""):
    return os.path.join(
        install_root_dir,
        platform or get_platform_name(),
        arch or get_arch_name(),
    )


if __name__ == "__main__":
    print("This is not a standalone script.")
    sys.exit(1)
