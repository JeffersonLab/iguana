#!/usr/bin/env bash

if [ $# -eq 0 ]; then
  echo """USAGE: $0 [package] [command(default=meson)]

  package:  the package name; since this varies between package manager
            repositories, we prefer to use the name from that used by the CI

  command:  meson   return a string for meson function \`dependency()\`

            ALA     return a URL for the Arch Linux Archive (ALA), for CI
                    https://archive.archlinux.org/

            src     return a URL of the source code
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
    [ "$cmd" = "src" ] && echo "ERROR: command '$cmd' is not used for '$dep'" >&2 && exit 1
    ;;
  yaml-cpp)
    result_meson='>=0.7.0'
    result_ala='https://archive.archlinux.org/packages/y/yaml-cpp/yaml-cpp-0.7.0-2-x86_64.pkg.tar.zst'
    [ "$cmd" = "src" ] && echo "ERROR: command '$cmd' is not used for '$dep'" >&2 && exit 1
    ;;
  root|ROOT)
    result_meson='>=6.28'
    [ "$cmd" = "ALA" ] && echo "ERROR: command '$cmd' is not used for '$dep'" >&2 && exit 1
    result_src='https://root.cern/download/root_v6.28.12.source.tar.gz'
    ;;
  ruby)
    result_meson='>=2.7.2'
    result_ala='https://archive.archlinux.org/packages/r/ruby/ruby-2.7.2-1-x86_64.pkg.tar.zst'
    [ "$cmd" = "src" ] && echo "ERROR: command '$cmd' is not used for '$dep'" >&2 && exit 1
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
  src)   echo $result_src   ;;
  *)
    echo "ERROR: command '$cmd' is unknown" >&2
    exit 1
    ;;
esac
