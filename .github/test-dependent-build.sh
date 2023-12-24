#!/bin/bash
# CI test of building iguana-dependent code

set -e

# args
[ $# -lt 1 ] && echo "USAGE: $0 [tool] [test args]..." >&2 && exit 2
tool=$1
shift

# source, build, and install directories
source_dir=examples/build_with_$tool
build_dir=build-dependent
install_dir=install-dependent
mkdir -p $install_dir
install_dir=$(realpath $install_dir)

# executable
test_executable=iguana-example-00-basic

# print and execute a command
exe() {
  echo "--------------------------------------------------"
  echo "$@"
  echo "--------------------------------------------------"
  "$@"
}

# build and test
case $tool in
  cmake)
    exe cmake -S $source_dir -B $build_dir
    exe cmake --build $build_dir
    exe cmake --install $build_dir --prefix $install_dir
    exe $install_dir/bin/$test_executable "$@"
    ;;
  make)
    pushd $source_dir
    exe make
    popd
    exe $source_dir/bin/$test_executable "$@"
    ;;
  meson)
    exe meson setup --prefix=$install_dir $build_dir $source_dir
    exe meson install -C $build_dir
    exe $install_dir/bin/$test_executable "$@"
    ;;
  *)
    echo "ERROR: unknown tool '$tool'" >&2
    exit 1
    ;;
esac
