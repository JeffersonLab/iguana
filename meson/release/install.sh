#!/bin/bash

set -e
set -u

sourceDir=$(realpath $(dirname ${BASH_SOURCE[0]:-$0})/../..)
version=$($sourceDir/meson/detect-version.sh $sourceDir)
echo """
Detected version $version
"""

if [ $# -ne 2 ]; then
  print_var() { echo "$@" | sed 's/:/\n      /g'; }
  echo """
  USAGE: $0 [BUILD_DIR] [INSTALL_PREFIX]

    BUILD_DIR       local build directory
    INSTALL_PREFIX  install to INSTALL_PREFIX/$version

  NOTE: ensure necessary dependencies are in:

    PKG_CONFIG_PATH:
      $(print_var ${PKG_CONFIG_PATH-})

    CMAKE_PREFIX_PATH:
      $(print_var ${CMAKE_PREFIX_PATH-})
  """
  exit 2
fi
buildDir=$1
installDir=$(realpath $2)/$version
nativeFile=$sourceDir/meson/release/native/release.ini
echo """
sourceDir  = $sourceDir
buildDir   = $buildDir
installDir = $installDir
nativeFile = $nativeFile
"""

exit
meson setup $buildDir $sourceDir --native-file=$nativeFile --prefix=$installDir
meson install -C $buildDir
# TODO: generate module file
