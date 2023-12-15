#!/bin/bash
# detect the version number from the git tag

echo "Detecting version number..."
version=0.0.0

pushd $(dirname $0)/..

git_tag=$(git describe --tags --abbrev=0)
if [ -n "$git_tag" ]; then
  version=$(echo $git_tag | sed 's;^v;;')
  echo "VERSION = $version"
else
  echo """WARNING: not a git repository
  setting VERSION to $version
  edit $(pwd)/.version to set the correct version""" >&2
fi

echo $version > .version

popd
