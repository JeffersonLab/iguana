#!/bin/bash
# CI test of building iguana-dependent code

set -e

# args
[ $# -lt 1 ] && echo "USAGE: $0 [tool] [test args]..." >&2 && exit 2
tool=$1
shift

# dependencies: assumed to be in `./<dependency>`
fmt_dep=$(realpath fmt)
iguana_dep=$(realpath iguana)
hipo_dep=$(realpath hipo)

# dependency resolution objects
pkg_config_path=(
  $fmt_dep/lib/pkgconfig
  $iguana_dep/lib/pkgconfig
)
cmake_prefix_path=(
  $hipo_dep
)
ld_library_path=(
  $hipo_dep/lib
  $fmt_dep/lib
  $iguana_dep/lib
)
hipo=$hipo_dep # needed for Makefile

# source, build, and install directories
source_dir=examples/build_with_$tool
build_dir=build-dependent
install_dir=install-dependent
mkdir -p $install_dir
install_dir=$(realpath $install_dir)

# executable
test_executable=iguana-example-00-basic

# join items in a list to a string delimited by $1
joinList() {
  d=$1
  shift
  echo "$@" | sed "s/ /$d/g"
}

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
    exe \
      cmake \
      -DCMAKE_PREFIX_PATH="$(joinList ';' ${cmake_prefix_path[*]} ${pkg_config_path[*]})" \
      -S $source_dir -B $build_dir
    exe cmake --build $build_dir
    exe cmake --install $build_dir --prefix $install_dir
    exe $install_dir/bin/$test_executable "$@"
    ;;
  make)
    pushd $source_dir
    exe \
      PKG_CONFIG_PATH=$(joinList ':' ${pkg_config_path[*]}) \
      HIPO=$hipo \
      make
    popd
    exe \
      LD_LIBRARY_PATH=$(joinList ':' ${ld_library_path[*]}) \
      $source_dir/bin/$test_executable "$@"
    ;;
  meson)
    exe \
      meson setup \
      --prefix=$install_dir \
      --Dcmake_prefix_path=$(joinList ',' ${cmake_prefix_path[*]}) \
      --Dpkg_config_path=$(joinList ',' ${pkg_config_path[*]}) \
      $build_dir $source_dir
    exe meson install -C $build_dir
    exe $install_dir/bin/$test_executable "$@"
    ;;
  *)
    echo "ERROR: unknown tool '$tool'" >&2
    exit 1
    ;;
esac
