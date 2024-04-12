#!/bin/bash
# source ROOT environment for CI
set -e
set -u

# parse arguments
if [ $# -ne 1 ]; then
  echo """
  USAGE: $0 [path/to/root]

  NOTE: this should only be used on GitHub CI runners;
  normal users should instead use 'thisroot.sh' directly
  """
  exit 2
fi

source $1/bin/thisroot.sh

echo "CLING_STANDARD_PCH=$CLING_STANDARD_PCH" >> $GITHUB_ENV
echo "CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH" >> $GITHUB_ENV
echo "DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH" >> $GITHUB_ENV
echo "JUPYTER_CONFIG_DIR=$JUPYTER_CONFIG_DIR" >> $GITHUB_ENV
echo "JUPYTER_PATH=$JUPYTER_PATH" >> $GITHUB_ENV
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH" >> $GITHUB_ENV
echo "LIBPATH=$LIBPATH" >> $GITHUB_ENV
echo "MANPATH=$MANPATH" >> $GITHUB_ENV
echo "PATH=$PATH" >> $GITHUB_ENV
echo "PYTHONPATH=$PYTHONPATH" >> $GITHUB_ENV
echo "ROOTSYS=$ROOTSYS" >> $GITHUB_ENV
echo "SHLIB_PATH=$SHLIB_PATH" >> $GITHUB_ENV
