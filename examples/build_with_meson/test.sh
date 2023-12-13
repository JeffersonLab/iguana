#!/bin/bash
# test building and running this example

set -e

# paths to dependencies
# - you will need to customize these
# - this script is used by CI, which assumes they are in `./<dependency>`
hipo_dep=$(realpath hipo)
iguana_dep=$(realpath iguana)

# source directory is where this script is found
source_dir=$(dirname $0)

# buildsystem directory
build_dir=build-with-meson-example

# installation prefix, which must be an absolute path
install_dir=install-with-meson-example
mkdir -p $install_dir
install_dir=$(realpath $install_dir)

# call `meson setup`
# - use build options `-Dcmake_prefix_path` and `-Dpkg_config_path` to specify dependency locations
#   (alternatively use corresponding environment variables)
meson setup \
  -Dcmake_prefix_path=$hipo_dep \
  -Dpkg_config_path=$iguana_dep/lib/pkgconfig \
  --prefix=$install_dir \
  $build_dir \
  $source_dir

# call `meson install`
meson install -C $build_dir

# run the executable
# - passes script arguments to the executable
$install_dir/bin/iguana-example-00-basic "$@"
