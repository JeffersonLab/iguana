#!/usr/bin/env bash
# generate a template algorithm from the example algorithm

set -e

namespace=clas12
installDir=src/iguana/algorithms/clas12

if [ $# -lt 1 ]; then
  echo """USAGE: $0 [algorithm class name] [namespace] [directory]

  [algorithm class name]: the name of your new algorithm
                          (required)

  [namespace]: the namespace (within the \`iguana\` namespace) of your new algorithm
               (default: '$namespace')
               - if you want the namespace to be \`iguana\` alone, set this to 'iguana'

  [directory]: the directory where your algorithm will be created
               (default: $installDir)
  """ >&2
  exit 2
fi

algo=$1
[ $# -ge 2 ] && namespace=$2
[ $# -ge 3 ] && installDir=$3

thisFile=${BASH_SOURCE[0]:-$0}
thisDir=$(cd $(dirname $thisFile) && pwd -P)
exName=ExampleAlgorithm

mkdir -p $installDir

for ext in h cc yaml; do
  outFile=$installDir/$algo.$ext
  cat $thisDir/$exName.$ext |\
    sed "s;$exName;$algo;g" |\
    sed "s;example::;$namespace::;g" |\
    sed "s;::example;::$namespace;g" |\
    sed "s;iguana::iguana;iguana;g" |\
    grep -v '// #' |\
    grep -vE '^# ' \
    > $outFile
  echo Created $outFile
done

echo """
Done. Now add '$algo.h' and '$algo.cc' to the appropriate meson.build file
"""
