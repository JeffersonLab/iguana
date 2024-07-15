#!/usr/bin/env bash
# create a new branch and commit changes from clang-format
set -e
set -u
if [ $# -ne 1 ]; then
  echo "USAGE: $0 [BUILD_DIR]" >&2
  exit 2
fi
builddir=$1
ref=auto-format-$(date +%s)
git checkout -b $ref
ninja -C $builddir clang-format
meson format --inplace --recursive
