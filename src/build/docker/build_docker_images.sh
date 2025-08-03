#!/bin/bash

# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set -e

scripts_dir=$(cd $(dirname $0) && pwd)
root_dir="${scripts_dir}/../../.."
cd ${root_dir}

image_name="redy"

docker build . -f ${scripts_dir}/Dockerfile -t ${image_name}:arch_linux
# docker build . -f ${scripts_dir}/ubuntu.dockerfile -t ${image_name}:ubuntu
