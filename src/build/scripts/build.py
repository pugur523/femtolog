#!/usr/bin/env python3

# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

import argparse
from itertools import product
import os
import sys

from femtolog_code_util import run_cpplint, run_clang_format
from concurrent.futures import ThreadPoolExecutor
from tabulate import tabulate
from time import time
from femtolog_build_util import (
    build_platform_dir,
    get_platform_name,
    get_arch_name,
    install_platform_dir,
    run_command,
    project_root_dir,
    project_src_dir,
)

supported_platforms = [
    "linux",
    "windows",
    "darwin",
    "mingw",
]
supported_architectures = [
    "x86_64",
    "amd64",
    "arm",
    "arm64",
]

build_types = ["debug", "release"]

build_modes = [
    "all",
    "all_options_matrix",
] + build_types


def select_best_toolchain(build_os, target_os):
    toolchains_dir = os.path.join(project_src_dir, "build", "cmake", "toolchains")
    toolchain_name = target_os

    if build_os == "windows":
        if target_os != "windows":
            # TODO: cygwin support
            print("Cross compile on windows is currently not supported.")
            return None
    elif build_os == "darwin":
        if target_os != "darwin":
            print("Cross compile on darwin is currently not supported.")
            return None
    elif build_os == "linux":
        if target_os == "windows" or target_os == "mingw":
            toolchain_name = "mingw"
        elif target_os == "darwin":
            print("Cross compile for darwin is currently not supported.")
            return None
    else:
        print("Unknown build os detected: ", build_os)
        return None

    return os.path.join(toolchains_dir, (toolchain_name + ".cmake"))


def build_with_all_option_combinations(
    target_platforms,
    archs,
    build_types,
    do_clang_tidy,
    fail_fast,
):
    options = {
        "-DFEMTOLOG_ENABLE_AVX2": ["false", "true"],
        "-DFEMTOLOG_ENABLE_SANITIZERS": ["false", "true"],
    }

    common_args = [
        "-DFEMTOLOG_BUILD_SHARED=true",
        "-DFEMTOLOG_BUILD_TESTING=true",
        "-DFEMTOLOG_BUILD_BENCHMARK=true",
        "-DFEMTOLOG_INSTALL_TESTING=false",
        "-DFEMTOLOG_INSTALL_BENCHMARK=false",
        "-DFEMTOLOG_ENABLE_RUN_TESTING_POST_BUILD=false",
        "-DFEMTOLOG_ENABLE_RUN_BENCHMARK_POST_BUILD=false",
        "-DFEMTOLOG_ENABLE_WARNINGS_AS_ERRORS=true",
        "-DFEMTOLOG_ENABLE_LTO=true",
        "-DFEMTOLOG_ENABLE_XRAY=false",
        "-DFEMTOLOG_ENABLE_NATIVE_ARCH=true",
        "-DFEMTOLOG_ENABLE_BUILD_REPORT=false",
        "-DFEMTOLOG_ENABLE_COVERAGE=false",
        "-DFEMTOLOG_ENABLE_OPTIMIZATION_REPORT=false",
    ]

    common_args_str = ",".join(common_args)

    option_keys = list(options.keys())
    option_values = list(options.values())

    raw_option_combinations = list(product(*option_values))

    def is_valid_combination(platform, build_type, opt_combo):
        opt_dict = dict(zip(option_keys, opt_combo))
        if build_type == "release" and opt_dict["-DFEMTOLOG_ENABLE_SANITIZERS"] == "true":
            return False
        return True

    all_combinations = [
        (platform, arch, build_type, opt_combo)
        for platform in target_platforms
        for arch in archs
        for build_type in build_types
        for opt_combo in raw_option_combinations
        if is_valid_combination(platform, build_type, opt_combo)
    ]

    headers = ["OS", "Arch", "BuildType"] + option_keys + ["TimeElapsed", "Result"]

    def build_single_combination(platform, arch, build_type, opt_values):
        variable_opts_str = ",".join(
            f"{k}={v.upper()}" for k, v in zip(option_keys, opt_values)
        )
        extra_args = common_args_str + "," + variable_opts_str
        print(f"starting {platform}-{arch}-{build_type}, {variable_opts_str}")
        start_time = time()
        try:
            result = build_project(
                target_platform=platform,
                target_arch=arch,
                build_type=build_type,
                do_clang_tidy=do_clang_tidy,
                build_async=True,
                install=False,
                package=False,
                verbose=False,
                extra_args=extra_args,
            )
            status = "✅" if result == 0 else "❌"
        except Exception as e:
            status = f"💥 ({e})"
        total_build_sec = time() - start_time
        minutes = int(total_build_sec // 60)
        seconds = int(total_build_sec % 60)
        return (
            [platform, arch, build_type]
            + list(opt_values)
            + [f"{minutes}m {seconds}s", status]
        )

    results = []
    print(f"Testing {len(all_combinations)} option combinations...")

    start_time = time()
    for platform, arch, build_type, opt_values in all_combinations:
        row = build_single_combination(platform, arch, build_type, opt_values)
        results.append(row)
        if fail_fast and row[-1] != "✅":
            print(tabulate(results, headers=headers, tablefmt="grid"))
            return 1

    print("\nBuild Result: \n")
    print(tabulate(results, headers=headers, tablefmt="grid"))

    ret = 0
    failures = [r for r in results if r[-1] != "✅"]
    if failures:
        print(f"\n{len(failures)} / {len(results)} combinations failed.")
        print(tabulate(failures, headers=headers, tablefmt="grid"))
        ret = 1

    total_sec = time() - start_time
    minutes = int(total_sec // 60)
    seconds = int(total_sec % 60)
    print(f"\nAll builds completed in {minutes} min {seconds} sec.")
    return ret


def build_project(
    target_platform: str,
    target_arch: str,
    build_type: str,
    do_clang_tidy: bool = False,
    build_async: bool = True,
    install: bool = False,
    package: bool = False,
    verbose: bool = False,
    extra_args: str = "",
):
    if target_platform not in supported_platforms:
        print(f"Unknown platform specified ({target_platform})")
        return 1
    if target_arch not in supported_architectures:
        print(f"Unknown architecture specified ({target_arch})")
        return 2

    build_dir = os.path.join(
        build_platform_dir(target_platform, target_arch), build_type
    )
    install_dir = os.path.join(
        install_platform_dir(target_platform, target_arch), build_type
    )

    print(
        f"Configuring project with CMake for {target_platform} ({target_arch}) in {build_type} mode..."
    )

    toolchain_file = select_best_toolchain(get_platform_name(), target_platform)
    if not toolchain_file:
        print("Compatible toolchain not found.")
        return -1

    args = [
        "--toolchain",
        toolchain_file,
        "--install-prefix",
        install_dir,
        "-D BUILD_DEBUG=" + ("true" if build_type == "debug" else "false"),
        "-D TARGET_OS_NAME=" + target_platform,
        "-D TARGET_ARCH=" + target_arch,
        "-D DO_CLANG_TIDY=" + ("true" if do_clang_tidy else "false"),
    ]
    if verbose:
        os.environ["VERBOSE"] = "true"
        args.extend(["-D FEMTOLOG_ENABLE_VERBOSE=true"])
    else:
        os.environ.pop("VERBOSE", "")
        args.extend(["-D FEMTOLOG_ENABLE_VERBOSE=false"])

    if extra_args:
        extra_args_list = extra_args.split(",")
        args.extend(extra_args_list)

    if get_platform_name() == "windows":
        llvm_dir = os.environ.get("LLVM_DIR", "C:/Program Files/LLVM")
        if os.path.exists(llvm_dir):
            args.extend(
                [
                    "-D LLVM_INCLUDE_DIRS=" + os.path.join(llvm_dir, "include"),
                    "-D LLVM_LIBRARY_DIRS=" + os.path.join(llvm_dir, "lib"),
                ]
            )
    elif target_platform == "windows" and get_platform_name() == "linux":
        llvm_mingw_dir = os.environ.get("LLVM_MINGW_DIR", "/opt/llvm-mingw*")
        llvm_w64_mingw32_dir = os.path.join(llvm_mingw_dir, "x86_64-w64-mingw32")

        if os.path.exists(llvm_w64_mingw32_dir):
            args.extend(
                [
                    "-D LLVM_INCLUDE_DIRS="
                    + os.path.join(llvm_w64_mingw32_dir, "include"),
                    "-D LLVM_LIBRARY_DIRS=" + os.path.join(llvm_w64_mingw32_dir, "lib"),
                ]
            )
            os.environ["PATH"] = (
                f'{os.environ["PATH"]}:{os.path.join(llvm_mingw_dir, "bin")}'
            )

    additional_include_dirs = os.environ.get("INCLUDE", "")
    additional_link_dirs = os.environ.get("LIB", "")
    args.extend(
        [
            "-D ADDITIONAL_INCLUDE_DIRECTORIES=" + additional_include_dirs,
            "-D ADDITIONAL_LINK_DIRECTORIES=" + additional_link_dirs,
        ]
    )

    configure_command = [
        "cmake",
        "-S",
        project_root_dir,
        "-B",
        build_dir,
        "-G",
        "Ninja",
    ]
    configure_command.extend(args)
    build_command = ["cmake", "--build", build_dir]
    install_command = ["cmake", "--install", build_dir]
    package_command = ["cmake", "--build", build_dir, "--target", "package"]

    if build_async:
        parallel_command = ["--parallel", os.cpu_count or 4]
        build_command.extend(parallel_command)
        package_command.extend(parallel_command)

    result = run_command(
        configure_command,
        cwd=project_root_dir,
    )
    if result.returncode != 0:
        print("cmake configure failed: ", result.returncode)
        return result.returncode

    result = run_command(build_command, cwd=project_root_dir)
    if result.returncode != 0:
        print("cmake build failed: ", result.returncode)
        return result.returncode

    if install:
        result = run_command(install_command, cwd=project_root_dir)
        if result.returncode != 0:
            print("cmake install failed: ", result.returncode)
            return result.returncode

    if package:
        result = run_command(package_command, cwd=project_root_dir)
        if result.returncode != 0:
            print("cmake package failed: ", result.returncode)
            return result.returncode

    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="build script.")
    parser.add_argument(
        "--build_mode",
        type=str,
        default="debug",
        choices=build_modes,
        help="comma-separated build types",
    )
    parser.add_argument(
        "--target_platforms",
        type=str,
        default=get_platform_name(),
        help="comma-separated target platforms",
    )
    parser.add_argument(
        "--target_archs",
        type=str,
        default=get_arch_name(),
        help="comma-separated target architectures",
    )
    parser.add_argument(
        "--clang_format",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="run clang-format before build",
    )
    parser.add_argument(
        "--clang_tidy",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="run clang-tidy analysis on build",
    )
    parser.add_argument(
        "--cpplint",
        action=argparse.BooleanOptionalAction,
        default=False,
        help="run cpplint before build",
    )
    parser.add_argument(
        "--build_async",
        action=argparse.BooleanOptionalAction,
        default=False,
        help="build all configs asynchronously",
    )
    parser.add_argument(
        "--install",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="install post build",
    )
    parser.add_argument(
        "--package",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="create package post build",
    )
    parser.add_argument(
        "--fail_fast",
        action=argparse.BooleanOptionalAction,
        default=False,
        help="return fast on build failure",
    )
    parser.add_argument(
        "--verbose",
        action=argparse.BooleanOptionalAction,
        default=False,
        help="verbose mode",
    )
    parser.add_argument(
        "--extra_args",
        type=str,
        default="",
        help='comma-separated extra arguments to pass to CMake (e.g. "-DOPTION=VALUE,-DXXX=YYY")',
    )

    args = parser.parse_args()

    if args.clang_format:
        run_clang_format(project_root_dir)

    if args.cpplint:
        run_cpplint(project_root_dir)

    target_platforms = [x for x in args.target_platforms.split(",") if x]
    target_archs = [x for x in args.target_archs.split(",") if x]
    target_build_types = (
        ["debug", "release"]
        if args.build_mode == "all" or args.build_mode == "all_options_matrix"
        else [x for x in args.build_mode.split(",") if x]
    )
    print("target platforms: ", ", ".join(target_platforms))
    print("target architectures: ", ", ".join(target_archs))
    print("target build types: ", ", ".join(target_build_types))

    if args.build_mode == "all_options_matrix":
        return build_with_all_option_combinations(
            target_platforms,
            target_archs,
            target_build_types,
            args.clang_tidy,
            args.fail_fast,
        )

    start_time = time()

    successfull_configs = []
    failed_configs = []

    build_tasks = list(product(target_platforms, target_archs, target_build_types))

    for target_platform, target_arch, build_type in build_tasks:
        assert target_platform in supported_platforms
        assert target_arch in supported_architectures
        assert build_type in build_types

    if args.build_async:

        def build_helper(task):
            platform, arch, build_type = task
            config_entry = {
                "target_platform": platform,
                "target_arch": arch,
                "build_type": build_type,
            }
            result = build_project(
                target_platform=platform,
                target_arch=arch,
                build_type=build_type,
                do_clang_tidy=args.clang_tidy,
                build_async=args.build_async,
                install=args.install,
                package=args.package,
                verbose=args.verbose,
                extra_args=args.extra_args,
            )

            return result, config_entry

        with ThreadPoolExecutor(max_workers=int(os.cpu_count() or 4 / 2)) as executor:
            results = list(executor.map(build_helper, build_tasks))
        for result, entry in results:
            if result != 0:
                if args.fail_fast:
                    return result
                else:
                    failed_configs.append(entry)
                    continue
            else:
                successfull_configs.append(entry)
    else:
        for target_platform, target_arch, build_type in build_tasks:
            if target_platform == "mingw":
                target_platform = "windows"
            config_entry = {
                "target_platform": target_platform,
                "target_arch": target_arch,
                "build_type": build_type,
            }
            result = build_project(
                target_platform=target_platform,
                target_arch=target_arch,
                build_type=build_type,
                do_clang_tidy=args.clang_tidy,
                build_async=args.build_async,
                install=args.install,
                package=args.package,
                verbose=args.verbose,
                extra_args=args.extra_args,
            )
            if result != 0:
                if args.fail_fast:
                    return result
                else:
                    failed_configs.append(config_entry)
                    continue
            successfull_configs.append(config_entry)

    if successfull_configs:
        print("\nSuccessfull Builds:")
        print(tabulate(successfull_configs, headers="keys", tablefmt="grid"))

    if failed_configs:
        print("\nFailed Builds:")
        print(tabulate(failed_configs, headers="keys", tablefmt="grid"))
        failed_counts = len(failed_configs)
        return failed_counts

    end_time = time()
    total_sec = end_time - start_time
    minutes = int(total_sec // 60)
    seconds = int(total_sec % 60)
    print(f"\nBuild completed in {minutes} min {seconds} sec.")

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv))
    except KeyboardInterrupt:
        print("\nAborted building process.")
