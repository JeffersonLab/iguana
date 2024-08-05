#!/usr/bin/env bash
# download the ROOT source code

set -euo pipefail

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [TAG]" >&2
  exit 1
fi

tag=$1
curl_cmd='curl --silent -L -H "Accept: application/vnd.github+json" -H "X-GitHub-Api-Version: 2022-11-28"'

echo "Download ROOT tag '$tag' from:"
case $tag in
  latest)
    url=$($curl_cmd https://api.github.com/repos/root-project/ROOT/releases/latest | jq -r '.tarball_url')
    ;;
  *)
    url=$($curl_cmd https://api.github.com/repos/root-project/ROOT/tags | jq -r '.[] | select (.name=="'$tag'") | .tarball_url')
    ;;
esac

echo "  $url"
[ -z "$url" ] && echo "ERROR: GitHub API call returned empty string, perhaps tag '$tag' does not exist?" && exit 1
echo "downloading ..."
echo wget -nv --no-check-certificate --output-document root.tar.gz $url
