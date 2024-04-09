#!/bin/bash
# dump the rpath of installed objects

set -e
set -u

if [ $# -ne 2 ]; then
  echo "USAGE: $0 [INSTALLATION_PREFIX] [OS/RUNNER]" >&2
  exit 2
fi
prefix=$1
os=$2

# binaries
objs=($(find $prefix -type f -name "iguana-*" | grep -vE '\.py$'))

# libraries and `get_rpath` function
case $os in
  ubuntu*|linux)
    objs+=($(find $prefix -type f -name "*.so"))
    get_rpath() { readelf -d $1 | grep RUNPATH; }
    ;;
  macos*|darwin)
    objs+=($(find $prefix -type f -name "*.dylib"))
    get_rpath() { otool -l $1 | grep LC_RPATH -A3; }
    ;;
  *)
    echo "ERROR: unknown OS '$os'" >&2
    exit 1
    ;;
esac

for obj in ${objs[@]}; do
  echo """
  ==================================================================================
  $obj
  =================================================================================="""
  get_rpath $obj
done
