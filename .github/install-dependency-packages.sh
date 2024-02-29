#!/bin/bash

set -e

##############################
# LINUX
##############################
GENERAL_PACKAGE_LIST_LINUX=(
  python
  gcc
  clang
  make
  cmake
  tree
  which
  pkgconf
  ninja
  meson
  gcovr           # for coverage
  python-pygments # for coverage report syntax colors
  llvm            # for `llvm-symbolizer`, for human-readable sanitizer results
)
IGUANA_PACKAGE_LIST_LINUX=(
  fmt
  yaml-cpp
)

##############################
# MACOS
##############################
GENERAL_PACKAGE_LIST_MACOS=(
  tree
  ninja
  meson
)
IGUANA_PACKAGE_LIST_MACOS=(
  fmt
  yaml-cpp
)

#############################################

if [ $# -ne 2 ]; then
  echo "USAGE: $0 [runner] [latest/minver]" >&2
  exit 2
fi
runner=$1
verset=$2
summary_file=pkg_summary.md
> $summary_file

this_script=${BASH_SOURCE[0]:-$0}
this_dir=$(cd $(dirname $this_script)/.. && pwd -P)

case $verset in
  latest) echo "[+] Prefer latest version for iguana-specific dependencies" ;;
  minver) echo "[+] Prefer minimum required version for iguana-specific dependencies" ;;
  *)
    echo "ERROR: unknown verset '$verset'" >&2
    exit 1
    ;;
esac

#############################################

info_pacman() {
  echo "| \`$1\` | $(pacman -Qi $1 | grep -Po '^Version\s*: \K.+') |" >> $summary_file
}

info_homebrew() {
  echo "| \`$1\` | $(brew info $1 | head -n1) |" >> $summary_file
}

#############################################

case $runner in

  ubuntu*)
    echo "[+] On Linux runner"
    echo "[+] UPDATING"
    pacman -Syu --noconfirm
    ### install latest version of general packages
    for pkg in ${GENERAL_PACKAGE_LIST_LINUX[@]}; do
      echo "[+] INSTALLING PACKAGE $pkg"
      pacman -S --noconfirm $pkg
      info_pacman $pkg
    done
    ### install either the latest or minver version of iguana-specific packages
    for pkg in ${IGUANA_PACKAGE_LIST_LINUX[@]}; do
      echo "[+] INSTALLING PACKAGE $pkg"
      case $verset in
        latest) pacman -S --noconfirm $pkg ;;
        minver) pacman -U --noconfirm $($this_dir/meson/minimum-version.sh $pkg ALA) ;;
      esac
      info_pacman $pkg
    done
    ;;

  macos*)
    [ "$verset" = "minver" ] && echo "ERROR: 'minver' not implemented for macOS" >&2 && exit 1
    echo "[+] On macOS runner"
    export NO_COLOR=1
    ### install the latest version of all packages
    for pkg in ${GENERAL_PACKAGE_LIST_MACOS[@]} ${IGUANA_PACKAGE_LIST_MACOS[@]}; do
      echo "[+] INSTALLING PACKAGE $pkg"
      brew install $pkg
      info_homebrew $pkg
    done
    ;;

  *)
    echo "ERROR: runner '$runner' is unknown to $0" >&2
    exit 1
    ;;

esac
