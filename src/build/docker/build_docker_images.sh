#!/bin/bash

set -e

scripts_dir=$(cd $(dirname $0) && pwd)
root_dir="${scripts_dir}/../../.."
cd ${root_dir}

image_name="femtolog"

docker build . -f ${scripts_dir}/ubuntu.dockerfile -t ${image_name}:ubuntu
docker build . -f ${scripts_dir}/arch_linux.dockerfile -t ${image_name}:arch_linux