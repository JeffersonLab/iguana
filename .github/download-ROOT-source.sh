#!/usr/bin/env bash
# download the ROOT source code

set -euo pipefail

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [TAG]" >&2
  exit 1
fi
tag=$1

get_payload() {
  curl --silent -L -H "Accept: application/vnd.github+json" -H "X-GitHub-Api-Version: 2022-11-28" $1
}

echo "================================="
case $tag in
  latest)
    payload=$(get_payload https://api.github.com/repos/root-project/ROOT/releases/latest)
    echo $payload | jq
    url=$(echo $payload | jq -r '.tarball_url')
    ;;
  *)
    payload=$(get_payload https://api.github.com/repos/root-project/ROOT/tags)
    echo $payload | jq
    url=$(echo $payload | jq -r '.[] | select (.name=="'$tag'") | .tarball_url')
    ;;
esac

echo "================================="
echo "Download ROOT tag '$tag' from:"
echo "  URL = $url"
echo "used jq version: $(jq --version)"
[ -z "$url" -o "$url" = "null" ] && echo "ERROR: GitHub API payload parsing failed, perhaps tag '$tag' does not exist?" && exit 1
echo "downloading ..."
# exec wget -nv --no-check-certificate --output-document root.tar.gz $url
