#!/usr/bin/env bash

set -e

##############################
# LINUX
##############################
GENERAL_PACKAGE_LIST_LINUX=(
  python
  gcc
  gcc-fortran
  clang
  make
  cmake
  tree
  wget
  git
  which
  jq
  pkgconf
  ninja
  meson
  llvm  # for `llvm-symbolizer`, for human-readable sanitizer results
  ### coverage
  python-colorlog
  python-pygments
  gcovr
  ### ROOT dependencies
  binutils
  libx11
  libxpm
  libxft
  libxext
  openssl
  gsl
  davix
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
  gcc # for gfortran
  ### ROOT dependencies
  binutils
  libx11
  libxpm
  libxft
  libxext
  openssl
  gsl
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
  brew info $1 > info.tmp
  echo "| \`$1\` | $(head -n1 info.tmp) |" >> $summary_file
  rm info.tmp
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
        minver) pacman -U --noconfirm --disable-download-timeout $($this_dir/meson/minimum-version.sh $pkg ALA) ;;
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
      brew install --quiet $pkg
      echo "[+] dump version number to summary"
      info_homebrew $pkg
    done
    ### link homebrew's gcc, for gfortran
    brew unlink gcc
    brew link gcc
    ;;

  *)
    echo "ERROR: runner '$runner' is unknown to $0" >&2
    exit 1
    ;;

esac
