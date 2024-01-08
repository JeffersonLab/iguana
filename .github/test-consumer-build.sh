#!/bin/bash
# CI test of building iguana-dependent code

set -e

# args
[ $# -ne 1 ] && echo "USAGE: $0 [tool]" >&2 && exit 2
tool=$1

# source, build, and install directories
source_dir=examples/build_with_$tool
build_dir=build-consumer
install_dir=install-consumer
mkdir -p $install_dir
install_dir=$(cd $install_dir && pwd -P)

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
    ;;
  make)
    pushd $source_dir
    exe make
    popd
    mkdir -pv $install_dir
    mv -v $source_dir/bin $install_dir/
    ;;
  meson)
    exe meson setup --prefix=$install_dir $build_dir $source_dir
    exe meson install -C $build_dir
    ;;
  *)
    echo "ERROR: unknown tool '$tool'" >&2
    exit 1
    ;;
esac
