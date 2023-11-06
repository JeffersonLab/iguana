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

msg() { echo "[+++] $*"; }
cmdSetup() { msg SETUP && meson setup --prefix $installDir $buildDir $sourceDir; }
cmdConfig() { msg CONFIGURE && meson configure $buildDir --prefix $installDir; }
cmdBuild() { msg BUILD && meson install -C $buildDir; }

if [ ! -d $buildDir ]; then
  cmdSetup
else
  cmdConfig || cmdSetup
fi
cmdBuild
