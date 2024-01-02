#!/bin/bash
# detect the version number from the latest git tag
set -e
[ $# -ne 1 ] && echo "USAGE: $0 [GIT_REPO]" >&2 && exit 2
cd $1
version=$(git describe --tags --abbrev=0 || echo '0.0.0')
echo $version | sed 's;^v;;'
