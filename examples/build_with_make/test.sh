#!/bin/bash
# test building and running this example

set -e

# paths to dependencies
# - you will need to customize these
# - this script is used by CI, which assumes they are in `./<dependency>`
hipo_dep=$(realpath hipo)
iguana_dep=$(realpath iguana)

# set pkg-config path
export PKG_CONFIG_PATH=$iguana_dep/lib/pkgconfig

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
# - passes script arguments to the executable
export LD_LIBRARY_PATH=$hipo_dep/lib:$iguana_dep/lib
$source_dir/bin/iguana-example-00-basic "$@"
