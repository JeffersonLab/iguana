#!/usr/bin/env bash
set -euo pipefail
if [ $# -ne 1 ]; then
  echo "USAGE: $0 [BUILD_DIRECTORY]" >&2
  exit 1
fi
builddir=$1
meson introspect --buildoptions $builddir | jq -r 'map(select(.section=="user") | del(.section,.machine))'
