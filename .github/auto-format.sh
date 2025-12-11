#!/usr/bin/env bash
set -euo pipefail

builddir=$(dirname $0)/../build

if [ -d $builddir ]; then
  ninja -C $builddir clang-format
else
  echo "$builddir does not exist, not auto-formatting"
  exit 0 # so pre-commit doesn't fail
fi

# meson format --inplace --recursive
