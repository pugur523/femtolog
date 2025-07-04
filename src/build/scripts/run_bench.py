#!/usr/bin/env python3

# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

import os
import subprocess
import datetime
import femtolog_build_util
import sys
import argparse
from pathlib import Path

scripts_dir = os.path.dirname(os.path.realpath(__file__))
project_root_dir = femtolog_build_util.project_root_dir
benchmark_results_dir = os.path.join(project_root_dir, "src/bench/results")

benchmark_exe_dir = os.path.join(
    femtolog_build_util.build_platform_dir(),
    "release/bin",
)
benchmark_executable = os.path.join(benchmark_exe_dir, "femtolog_bench")


def parse_args():
    parser = argparse.ArgumentParser(
        description="Summarize Google Benchmark JSON output with percentiles.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    parser.add_argument(
        "--format", action="store_true", help="Format results after running benchmarks."
    )
    parser.add_argument(
        "--plot", action="store_true", help="Generate bar chart of percentiles."
    )

    return parser.parse_args()


def main():
    args = parse_args()
    current_datetime = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

    if not os.path.isfile(benchmark_executable):
        print("benchmark executable not found")
        return 1

    result_json = os.path.join(benchmark_results_dir, f"{current_datetime}.json")

    command = [
        benchmark_executable,
        "--benchmark_out=" + result_json,
        "--benchmark_out_format=json",
        "--benchmark_repetitions=5",
    ]

    subprocess.run(command, check=True)
    print(f"Benchmark results saved to {result_json}")

    if args.format:
        import femtolog_format_results

        plot_target = os.path.join(benchmark_results_dir, f"{current_datetime}.png")
        femtolog_format_results.format_result(
            Path(result_json),
            None,
            None,
            "real_time",
            [50, 75, 90, 95, 99, 99.9],
            True,
            Path(plot_target),
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())
