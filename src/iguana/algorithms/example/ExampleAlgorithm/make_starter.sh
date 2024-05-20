#!/usr/bin/env bash
# generate a starter algorithm from the example algorithm

set -e

namespace=clas12
if [ $# -lt 1 ]; then
  echo """USAGE: $0 [ALGORITHM_CLASS_NAME] [NAMESPACE]

  [ALGORITHM_CLASS_NAME]: the name of your new algorithm
                          (required)

  [NAMESPACE]: the namespace (within the \`iguana\` namespace) of your new algorithm
               (default: '$namespace')
               - if you want the namespace to be \`iguana\` alone, set this to 'iguana'
  """ >&2
  exit 2
fi

algo=$1
[ $# -ge 2 ] && namespace=$2

thisFile=${BASH_SOURCE[0]:-$0}
thisDir=$(cd $(dirname $thisFile) && pwd -P)
exName=ExampleAlgorithm

installDir=$thisDir/../../$namespace/$algo
mkdir -pv $installDir

for file in Algorithm.h Algorithm.cc Config.yaml; do
  outFile=$installDir/$file
  cat $thisDir/$file |\
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
Done. Generated starter algorithm in $(cd $installDir && pwd)

To compile it, update $(cd $thisDir/../.. && pwd)/meson.build
"""
