#!/bin/bash

# automation for deploying new releases
# this script is for maintainers, not users; users should
# follow installation documentation instead

set -e
set -u

sourceDir=$(realpath $(dirname ${BASH_SOURCE[0]:-$0})/../..)
version=$($sourceDir/meson/detect-version.sh $sourceDir)
echo """
Detected version $version"""

if [ $# -ne 2 ]; then
  print_var() { echo "$@" | sed 's/:/\n      /g'; }
  echo """
  USAGE: $0 [BUILD_PREFIX] [INSTALL_PREFIX]

    BUILD_PREFIX    build in BUILD_PREFIX-$version
    INSTALL_PREFIX  install to INSTALL_PREFIX/$version

  NOTE: ensure necessary dependencies are in:

    PKG_CONFIG_PATH:
      $(print_var ${PKG_CONFIG_PATH-})

    CMAKE_PREFIX_PATH:
      $(print_var ${CMAKE_PREFIX_PATH-})
  """
  exit 2
fi
buildDir=$1-$version
installDir=$(realpath $2)/$version
nativeFile=$sourceDir/meson/release/native/release.ini
echo """
sourceDir  = $sourceDir
buildDir   = $buildDir
installDir = $installDir
"""
echo "nativeFile = $nativeFile:"
cat $nativeFile
printf "\nProceed with installation? [y/N] "
read proceed
case $proceed in
  y|Y|yes)
    echo "Proceeding with installation..."
    ;;
  *)
    echo "Abort."
    exit 1
    ;;
esac

meson setup $buildDir $sourceDir --native-file=$nativeFile --prefix=$installDir
meson install -C $buildDir
