#!/bin/bash

set -e

sourceDir=$(cd $(dirname ${BASH_SOURCE[0]:-$0}) && pwd -P)
buildDir=build-iguana
[ $# -gt 0 ] && installDir=$1 || installDir=iguana
[[ ! "$installDir" =~ ^/ ]] && installDir=$(pwd)/$installDir
echo """
sourceDir  = $sourceDir
buildDir   = $buildDir
installDir = $installDir
"""

[ ! -d $buildDir ] && 
  meson setup --prefix $installDir $buildDir $sourceDir ||
  meson configure $buildDir --prefix $installDir
meson install -C $buildDir
