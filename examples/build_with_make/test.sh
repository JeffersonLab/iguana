#!/bin/bash
# test building and running this example

set -e

# paths to dependencies
# - you will need to customize these
# - if they are installed on your system, (e.g., `fmt`), you may not need to specify them
# - this script is used by CI, which assumes they are in `./<dependency>`
hipo_dep=$(realpath hipo)
fmt_dep=$(realpath fmt)
iguana_dep=$(realpath iguana)

# set pkg-config path
export PKG_CONFIG_PATH=$fmt_dep/lib/pkgconfig:$iguana_dep/lib/pkgconfig

# hipo doesn't use pkg-config; instead just set a variable to its installation prefix
export HIPO=$hipo_dep

# source directory is where this script is found
source_dir=$(dirname $0)

# build
pushd $source_dir
make
popd

# run the executable
# - since we didn't set the rpath in the Makefile, we must use LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$hipo_dep/lib:$fmt_dep/lib:$iguana_dep/lib
$source_dir/bin/iguana-example-00-basic
