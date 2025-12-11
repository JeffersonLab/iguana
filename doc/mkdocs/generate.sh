#!/usr/bin/env bash

set -euo pipefail

# parse arguments
if [ $# -lt 2 ]; then
  echo "USAGE: $0 [BUILD_DIR] [OUTPUT_DIR] [VERSIONS]..."
  exit 2
fi
src_dir=$(dirname $0)
build_dir=$(realpath $1)
output_dir=$(realpath $2)
shift
shift
versions=($@)

# make output directories
mkdir -pv $build_dir
mkdir -pv $output_dir

# populate build directory
cp -r $src_dir/docs $build_dir/
cp -r $src_dir/mkdocs.yaml $build_dir/
cp -r $src_dir/../logo*.png $build_dir/docs/

# replace @guides@ with table of version numbers and links
version_table="| **User Guide** | **Notes** |\n"
version_table+="| --- | --- |"
for ver in ${versions[@]}; do
  note=''
  name=$ver
  if [ "$ver" == "main" ]; then
    name='Latest'
    ver='latest'
    note='Applies to _only_ the current `main` branch; it may _differ_ from the latest _numbered_ version below'
  fi
  version_table+="\n| <b><a href=$ver/index.html>$name</a></b> | $note |"
done
echo "version_table=$version_table"
sed -i "/@guides@/c\\${version_table}" $build_dir/docs/index.md

# run mkdocs
mkdocs build --config-file $build_dir/mkdocs.yaml --site-dir $output_dir
