#!/usr/bin/env python3

# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

import os
import sys

from build_util import (
    is_installed,
    run_command,
    project_root_dir,
)


def run_cpplint(src_dir, cpplint_cfg=""):
    if not is_installed("cpplint"):
        print("cpplint not found. please install cpplint and add it to $PATH.")
        return 1
    if not cpplint_cfg:
        cpplint_cfg = "CPPLINT.cfg"
    result = run_command(
        [
            "python3",
            "-m",
            "cpplint",
            "--exclude=src/third_party",
            "--recursive",
            "--config=" + cpplint_cfg,
            "./src",
        ],
        cwd=src_dir,
    )
    if result.returncode != 0:
        print("cpplint failed: ", result.returncode)
    return result.returncode


def run_clang_format(root_dir, exclude_dirs=[]):
    if not exclude_dirs:
        exclude_dirs = ["third_party"]

    if not is_installed("clang-format"):
        print(
            "clang-format not found. please install clang-format and add it to $PATH."
        )
        return 1
    error = ""
    for dirpath, _, filenames in os.walk(os.path.join(root_dir, "src")):
        # skip exclude directories
        should_skip = False
        for exclude_dir in exclude_dirs:
            if exclude_dir in dirpath.split(os.sep):
                should_skip = True
                break

        if should_skip:
            continue

        for filename in filenames:
            if filename.endswith(".cc") or filename.endswith(".h"):
                file_path = os.path.join(dirpath, filename)
                print("start clang-format for ", file_path)
                result = run_command(
                    [
                        "clang-format",
                        file_path,
                        "-i",
                        "--fail-on-incomplete-format",
                        "--ferror-limit=1",
                        "--sort-includes",
                    ],
                    cwd=root_dir,
                )
                if result.returncode != 0:
                    error += f"clang-format failed for {file_path}\n"
    if error != "":
        print(error)
        return 1
    return 0


def show_help():
    print(f'usage: {os.path.basename(__file__)} <cpplint|clang-format>"')


def main(argv):
    if len(argv) <= 1:
        show_help()
        return 1

    action = argv[1]
    if action == "cpplint":
        return run_cpplint(project_root_dir)
    if action == "clang-format":
        return run_clang_format(project_root_dir)
    else:
        print("unknown action specified: ", action)
        show_help()
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
