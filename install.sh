#!/bin/bash

sourceDir=$(cd $(dirname ${BASH_SOURCE[0]:-$0}) && pwd -P)
buildDir=build-iguana
[ $# -gt 0 ] && installDir=$1 || installDir=iguana
[[ ! "$installDir" =~ ^/ ]] && installDir=$(pwd)/$installDir
echo """
sourceDir  = $sourceDir
buildDir   = $buildDir
installDir = $installDir"""

if [ ! -d $buildDir ]; then
  meson setup --prefix $installDir $buildDir $sourceDir
else
  meson configure $buildDir --prefix $installDir
fi

meson install -C $buildDir
