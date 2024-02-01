#!/bin/bash

set -e

PACKAGE_LIST_ARCH_LINUX=(
  ### general dependencies
  python
  gcc
  make
  cmake
  tree
  pkgconf
  ### iguana dependencies
  ninja
  meson
)

PACKAGE_LIST_MACOS=(
  ### general dependencies
  tree
  ### iguana dependencies
  ninja
  meson
  fmt
)

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [runner]" >&2
  exit 2
fi
runner=$1
summary_file=pkg_summary.md
> $summary_file

case $runner in

  ubuntu*)
    echo "[+] On Linux runner"
    echo "[+] UPDATING"
    pacman -Syu --noconfirm
    for pkg in ${PACKAGE_LIST_ARCH_LINUX[@]}; do
      echo "[+] INSTALLING PACKAGE $pkg"
      pacman -S --noconfirm $pkg
      echo "| \`$pkg\` | $(pacman -Qi $pkg | grep -Po '^Version\s*: \K.+') |" >> $summary_file
    done
    echo "[+] TEST"
    pacman -U https://archive.archlinux.org/packages/f/fmt/fmt-9.1.0-4-x86_64.pkg.tar.zst
    ;;

  macos*)
    echo "[+] On macOS runner"
    export NO_COLOR=1
    for pkg in ${PACKAGE_LIST_MACOS[@]}; do
      echo "[+] INSTALLING PACKAGE $pkg"
      brew install $pkg
      echo "| \`$pkg\` | $(brew info $pkg | head -n1) |" >> $summary_file
    done
    ;;

  *)
    echo "ERROR: runner '$runner' is unknown to $0" >&2
    exit 1
    ;;

esac
