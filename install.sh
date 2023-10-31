#!/bin/bash

set -e

sourceDir=$(cd $(dirname ${BASH_SOURCE[0]:-$0}) && pwd)
buildDir=$(pwd)/build-iguana
[ $# -gt 0 ] && installDir=$1 || installDir=iguana
[[ ! "$installDir" =~ ^/ ]] && installDir=$(pwd)/$installDir
cat << EOF
source:       $sourceDir
build:        $buildDir
installation: $installDir
EOF

[ ! -d $buildDir ] && 
  meson setup --prefix $installDir $buildDir $sourceDir ||
  meson configure $buildDir --prefix $installDir
meson install -C $buildDir
