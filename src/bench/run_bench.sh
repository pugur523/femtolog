#!/bin/bash

# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set -e

scripts_dir=$(cd "$(dirname "$0")" && pwd)

project_root_dir=${scripts_dir}/../..
results_dir=${scripts_dir}/results
cd ${project_root_dir}

benchmark_dir=./out/build/linux/x86_64/release/bin
benchmark_executable=${benchmark_dir}/femtolog_bench

current_datetime=$(date '+%Y-%m-%d_%H-%M-%S')

# ${benchmark_executable} --benchmark_out=${results_dir}/${current_datetime}.txt --benchmark_out_format=console
${benchmark_executable} --benchmark_out=${results_dir}/${current_datetime}.json --benchmark_out_format=json --benchmark_repetitions=5
# ${benchmark_executable} --benchmark_out=${results_dir}/${current_datetime}.csv --benchmark_out_format=csv