#!/bin/bash

if [ $# -eq 0 ]; then
  echo """USAGE: $0 [package] [command(default=meson)]

  package:  the package name; since this varies between package manager
            repositories, we prefer to use the name from that used by the CI

  command:  meson   return a string for meson function \`dependency()\`

            ALA     return a URL for the Arch Linux Archive (ALA), for CI
                    https://archive.archlinux.org/
  """
  exit 2
fi
dep=$1
[ $# -ge 2 ] && cmd=$2 || cmd=meson

#############################################

case $dep in
  fmt)
    result_meson='>=9.1.0'
    result_ala='https://archive.archlinux.org/packages/f/fmt/fmt-9.1.0-4-x86_64.pkg.tar.zst'
    ;;
  *)
    echo "ERROR: dependency '$dep' is unknown" >&2
    exit 1
    ;;
esac

#############################################

case $cmd in
  meson) echo $result_meson ;;
  ALA)   echo $result_ala   ;;
  *)
    echo "ERROR: command '$cmd' is unknown" >&2
    exit 1
    ;;
esac
