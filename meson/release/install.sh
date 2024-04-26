#!/bin/bash

# automation for deploying new releases
# this script is for maintainers, not users; users should
# follow installation documentation instead

set -e
set -u

msg() {
  echo """
==================================================================================
$@
=================================================================================="""
}

print_var() {
  echo "$@" | sed 's/:/\n  /g'
}

print_dep_vars() {
  echo """
PKG_CONFIG_PATH:
  $(print_var ${PKG_CONFIG_PATH-})

CMAKE_PREFIX_PATH:
  $(print_var ${CMAKE_PREFIX_PATH-})"""
}

sourceDir=$(realpath $(dirname ${BASH_SOURCE[0]:-$0})/../..)
version=$($sourceDir/meson/detect-version.sh $sourceDir)
msg "Detected version $version"

if [ $# -ne 3 ]; then
  echo """
  USAGE: $0 [BUILD_PREFIX] [INSTALL_PREFIX] [TEST_HIPO_FILE]

    BUILD_PREFIX    build in BUILD_PREFIX-$version
    INSTALL_PREFIX  install to INSTALL_PREFIX/$version
    TEST_HIPO_FILE  a sample HIPO file, for running tests
  """
  print_dep_vars
  exit 2
fi
buildDir=$1-$version
installDir=$(realpath $2)/$version
testFile=$(realpath $3)
nativeFile=$sourceDir/meson/release/native/release.ini
echo """
sourceDir  = $sourceDir
buildDir   = $buildDir
installDir = $installDir
testFile   = $testFile"""
print_dep_vars
msg "nativeFile = $nativeFile:"
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

msg "meson setup"
meson setup $buildDir $sourceDir \
  --native-file=$nativeFile      \
  --prefix=$installDir           \
  -Dtest_data_file=$testFile
msg "meson install"
meson install -C $buildDir
msg "meson test"
meson test -C $buildDir

msg "Done installation"
echo """
  prefix:      $installDir

  moduleFile:  $buildDir/$version
"""
msg "Next steps:"
echo """
- [ ] copy the module file to the correct location
- [ ] module switch iguana/$version
- [ ] try running installed binaries
"""
