#!/bin/bash
# test building dependent builds

set -e

# args
[ $# -lt 1 ] && echo "USAGE: $0 [tool] [test args]..." >&2 && exit 2
tool=$1
shift

# dependencies
fmt_dep=$(realpath fmt)
iguana_dep=$(realpath iguana)
hipo_dep=$(realpath hipo)
export PKG_CONFIG_PATH=$fmt_dep/lib/pkgconfig:$iguana_dep/lib/pkgconfig
export HIPO=$hipo_dep
export CMAKE_PREFIX_PATH=$HIPO

# source, build, and install directories
source_dir=examples/build_with_$tool
build_dir=build-dependent
install_dir=install-dependent
mkdir -p $install_dir
install_dir=$(realpath $install_dir)

# executable
test_executable=iguana-example-00-basic

# build and test
case $tool in
  cmake)
    cmake -S $source_dir -B $build_dir -DCMAKE_INSTALL_PREFIX=$install_dir
    cmake --build $build_dir
    cmake --install $build_dir
    $install_dir/bin/$test_executable "$@"
    ;;
  make)
    pushd $source_dir
    make
    popd
    export LD_LIBRARY_PATH=$hipo_dep/lib:$fmt_dep/lib:$iguana_dep/lib
    $source_dir/bin/$test_executable "$@"
    ;;
  meson)
    meson setup --prefix=$install_dir $build_dir $source_dir
    meson install -C $build_dir
    $install_dir/bin/$test_executable "$@"
    ;;
  *)
    echo "ERROR: unknown tool '$tool'" >&2
    exit 1
    ;;
esac
