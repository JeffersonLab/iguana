#!/bin/bash
# source ROOT environment for CI
set -e
set -u

# parse arguments
if [ $# -lt 2 ]; then
  echo """
  USAGE: $0 [path/to/root] [runner] [OPTIONS]...

  OPTIONS:
             ld   append ld path

  NOTE: this should only be used on the CI
  """
  exit 2
fi
root_path=$1
runner=$2
shift
shift
set_ld_path=false
for arg in "$@"; do
  [ "$arg" = "ld" ] && set_ld_path=true
done

thisroot=$root_path/bin/thisroot.sh
if [ ! -f $thisroot ]; then
  echo "ERROR: 'thisroot.sh' is not found at $thisroot" >&2
  exit 1
fi

source $thisroot

echo PATH=$PATH >> $GITHUB_ENV
if $set_ld_path; then
  case "$runner" in
    macos-latest) echo DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH >> $GITHUB_ENV ;;
    *)            echo LD_LIBRARY_PATH=$LD_LIBRARY_PATH     >> $GITHUB_ENV ;;
  esac
fi
