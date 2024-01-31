#!/bin/bash

set -e

PACKAGE_LIST_MACOS=(
  fmt
  yaml-cpp
  tree
)

PACKAGE_LIST_LINUX=(
  fmt
  yaml-cpp
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
    for pkg in ${PACKAGE_LIST_LINUX[@]}; do
      echo "[+] INSTALLING $pkg"
      sudo pacman -Syu --noconfirm $pkg
      echo "| \`$pkg\` | $(pacman -Qi $pkg | grep -Po '^Version\s*: \K.+') |" >> $summary_file
    done
    ;;

  macos*)
    echo "[+] On macOS runner"
    export NO_COLOR=1
    for pkg in ${PACKAGE_LIST_MACOS[@]}; do
      echo "[+] INSTALLING $pkg"
      brew install $pkg
      if [ ! "$pkg" = "tree" ]; then
        echo "| \`$pkg\` | $(brew info $pkg | head -n1) |" >> $summary_file
      fi
    done
    ;;

  *)
    echo "ERROR: runner '$runner' is unknown to $0" >&2
    exit 1
    ;;

esac
