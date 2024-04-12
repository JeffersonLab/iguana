#!/bin/bash
# source ROOT environment for CI
set -e
set -u

# parse arguments
if [ $# -lt 2 ]; then
  echo """
  USAGE: $0 [path/to/root]

  NOTE: this should only be used on GitHub CI runners;
  normal users should instead use 'thisroot.sh' directly
  """
  exit 2
fi

source $1/bin/thisroot.sh

export CLING_STANDARD_PCH=$CLING_STANDARD_PCH >> $GITHUB_ENV
export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH >> $GITHUB_ENV
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH >> $GITHUB_ENV
export JUPYTER_CONFIG_DIR=$JUPYTER_CONFIG_DIR >> $GITHUB_ENV
export JUPYTER_PATH=$JUPYTER_PATH >> $GITHUB_ENV
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH >> $GITHUB_ENV
export LIBPATH=$LIBPATH >> $GITHUB_ENV
export MANPATH=$MANPATH >> $GITHUB_ENV
export PATH=$PATH >> $GITHUB_ENV
export PYTHONPATH=$PYTHONPATH >> $GITHUB_ENV
export ROOTSYS=$ROOTSYS >> $GITHUB_ENV
export SHLIB_PATH=$SHLIB_PATH >> $GITHUB_ENV
