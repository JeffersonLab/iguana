#!/usr/bin/env bash
# workaround for https://github.com/mesonbuild/meson/issues/5374
# to install Fortran .mod files

if [ $# -ne 2 ]; then
  echo "USAGE: $0 [BUILD_DIR] [INSTALL_DIR]" >&2
  exit 2
fi
buildDir=$1
installDir=$2

modFiles=($(find $buildDir/src/iguana -name "iguana_*.mod"))
if [ ${#modFiles[@]} -eq 0 ]; then
  echo "ERROR: no Fortran .mod files found" >&2
  exit 1
fi

mkdir -p $installDir
for modFile in ${modFiles[@]}; do
  cp -v $modFile $installDir/
done
