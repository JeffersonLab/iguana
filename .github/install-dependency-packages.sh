#!/bin/bash

set -e

PACKAGE_LIST_MACOS=(
  fmt
  yaml-cpp
  tree
)

PACKAGE_LIST_LINUX=(
  libyaml-cpp-dev
)

##################################

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [runner]" >&2
  exit 2
fi
runner=$1

##################################

summary_file=pkg_summary.md
> $summary_file

if [[ "$runner" =~ "macos" ]]; then
  echo "[+] On macOS runner"
  for pkg in ${PACKAGE_LIST_MACOS[@]}; do
    echo "[+] INSTALLING $pkg"
    brew install $pkg
    if [ ! "$pkg" = "tree" ]; then
      echo "| \`$pkg\` | $(brew info $pkg | head -n1) |" >> $summary_file
    fi
  done
elif [[ "$runner" =~ "ubuntu" ]]; then
  echo "[+] On Linux runner"
  echo "[+] Updating apt"
  sudo apt update
  sudo apt upgrade
  for pkg in ${PACKAGE_LIST_LINUX[@]}; do
    echo "[+] INSTALLING $pkg"
    sudo apt -y install $pkg
    echo "| \`$pkg\` | $(apt show $pkg | grep -E '^Version:') |" >> $summary_file
  done
else
  echo "ERROR: runner '$runner' is unknown to $0" >&2
  exit 1
fi
