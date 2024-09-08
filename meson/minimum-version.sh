#!/usr/bin/env bash

set -euo pipefail

cmd=ver
ala_mirror='https://archive.archlinux.org'
if [ $# -eq 0 ]; then
  echo """USAGE: $0 [package] [command] [ALA_mirror]

  package:  the package name; since this varies between package manager
            repositories, we prefer to use the name from that used by the CI

  command:
    choices:  ver         return the minimum version number
              ALA         return a URL for the Arch Linux Archive (ALA), for CI
              tag         return a git tag for the minimum version's source code
              tag_latest  return the latest git tag (that we test)
    default: $cmd

  ALA_mirror:  the ALA mirror, for command 'ALA'
    default: $ala_mirror

  """
  exit 2
fi
dep=$1
[ $# -ge 2 ] && cmd=$2
[ $# -ge 3 ] && ala_mirror=$3

not_used() {
  [ "$cmd" = "$1" ] && echo "ERROR: command '$cmd' is not used for '$dep'" >&2 && exit 1 || true
}

#############################################

case $dep in
  fmt)
    result_ver='9.1.0'
    result_ala="$ala_mirror/packages/f/fmt/fmt-9.1.0-4-x86_64.pkg.tar.zst"
    not_used 'tag'
    not_used 'tag_latest'
    ;;
  yaml-cpp)
    result_ver='0.7.0'
    result_ala="$ala_mirror/packages/y/yaml-cpp/yaml-cpp-0.7.0-2-x86_64.pkg.tar.zst"
    not_used 'tag'
    not_used 'tag_latest'
    ;;
  root|ROOT)
    result_ver='6.28.10'
    [ "$cmd" = "tag" ] && result_tag='v6-28-10' || result_tag='v6-32-04' # minver tag || maxver tag
    not_used 'ALA'
    ;;
  ruby)
    result_ver='2.7.2'
    not_used 'ALA'
    not_used 'tag'
    not_used 'tag_latest'
    ;;
  *)
    echo "ERROR: dependency '$dep' is unknown" >&2
    exit 1
    ;;
esac

#############################################

case $cmd in
  ver) echo $result_ver ;;
  ALA) echo $result_ala ;;
  tag|tag_latest) echo $result_tag ;;
  *)
    echo "ERROR: command '$cmd' is unknown" >&2
    exit 1
    ;;
esac
